    ____  _   _
    || \\ \\ //
    ||_//  )x(
    || \\ // \\ 2020.10
    a minimalist forth for nga

*Rx* (*retro experimental*) is a minimal Forth implementation
for the Nga virtual machine. Like Nga this is intended to be
used within a larger supporting framework adding I/O and other
desired functionality.

## General Notes on the Source

Rx is developed using a literate tool called *unu*. This allows
easy extraction of fenced code blocks into a separate file for
later compilation. I've found the use of a literate style to be
very beneficial as it makes it easier for me to keep the code
and commentary in sync, and helps me to approach development in
a more structured manner.

This source is written in Muri, an assembler for Nga.

Before going on, I should explain a bit about Nga and Muri.

Nga provides a MISC inspired virtual machine for a dual stack
architecture. There are 30 instructions, with up to four packed
into each memory location (*cell*). The instructions are:

    0 nop   5 push  10 ret   15 fetch 20 div   25 zret
    1 lit   6 pop   11 eq    16 store 21 and   26 halt
    2 dup   7 jump  12 neq   17 add   22 or    27 ienum
    3 drop  8 call  13 lt    18 sub   23 xor   28 iquery
    4 swap  9 ccall 14 gt    19 mul   24 shift 29 iinvoke

I won't explain them here, but if you're familiar with Forth,
it should be pretty easy to figure out.

Packing of instructions lets me save space, but does require a
little care. Instructions that modify the instruction pointer
should be followed by NOP. These are: JUMP, CALL, CCALL,
RETURN, and ZRET. Additionally, if the instruction bundle
contains a LIT, a value must be in the following cell. (One for
each LIT in the bundle)

The reason for this relates to how Nga processes the opcodes.
To illustrate, assume a stack with a couple of values:

    #1 #2

And a function that consumes two values before returning a
some new ones:

    : function
    i mulire..
    d 3

(In the `i` line, the instructions are mul, lit, return, and
nop).

If we were to use this along with an instruction bundle (the
line starting with `i`) like:

    : test
    i licaadre
    r function

(In the `i` line, the instructions are lit, call, add, and
return).

On running the code in `test` Nga will:

    (1) push a pointer to `function` to the stack
    (2) setup a call to the function
    (3) add the top values on the stack (#1 #2),
        leaving a single value (#3)
    (4) setup a return to the caller

At this point the bundle is done, so control goes to the
called function. But we now have only one value on the stack,
so the stack underflows and Nga will crash.

It's not forbidden by my VM implementation, but John's Impexus
implementation differs here in that it does not process the
instructions after jump, call, ccall, ret, or zret. I really
recommend making sure your bundles don't contain anything after
these instructions to keep things portable and predictable in
behavior.

Muri uses the first two characters of each instruction name
when composing the bundles, with NOP being named as two dots.

So:

    lit lit add nop

Is a bundle named:

    liliad..

And with two `li` instructions, must be followed by two values.

Muri uses a directive in the first line to tell it what to
expect. Valid directives are:

    i   instruction bundle
    d   decimal value
    r   reference to label
    :   label
    s   zero terminated string

## In the Beginning...

Nga expects code to start with a jump to the main entry point.
Rx doesn't really have a main entry point (the top level loop
is assumed to be part of the interface layer), but I allocate
the space for a jump here anyway. This makes it possible to
patch the entry point later, if using an interface that adds
the appropriate I/O functionality.

~~~
i liju....
d -1
~~~

With this, it's time to allocate some data elements. These are
always kept in known locations after the initial jump to ensure
that they can be easily identified and interfaced with external
tools. This is important as Nga allows for a variety of I/O
models to be implemented and I don't want to tie Rx into any
one specific model.

Here's the initial memory map:

| Offset | Contains                    |
| ------ | --------------------------- |
| 0      | lit call nop nop            |
| 1      | Pointer to main entry point |
| 2      | Dictionary                  |
| 3      | Heap                        |
| 4      | RETRO version               |

~~~
: Dictionary
r 9999

: Heap
d 1536

: Version
d 202010
~~~

Both of these are pointers. `Dictionary` points to the most
recent dictionary entry. (See the *Dictionary* section at the
end of this file.) `Heap` points to the next free address.
This is hard coded to an address beyond the end of the Rx
kernel. I adjust this as needed if the kernel grows or shinks
significantly. See the *Interpreter & Compiler* section for
more on this.

## Nga Instruction Set

As mentioned earlier, Nga provides 30 instructions. Rx begins
the actual coding by assigning most of these to a separate
function. These are not intended for direct use; the compiler
will fetch the opcode values to use from these functions when
compiling. Many will also be exposed in the initial dictionary.

~~~
: _nop
i ........
i re......

: _lit
i li......
i re......

: _dup
i du......
i re......

: _drop
i dr......
i re......

: _swap
i sw......
i re......

: _push
i pu......
i re......

: _pop
i po......
i re......

: _jump
i ju......
i re......

: _call
i ca......
i re......

: _ccall
i cc......
i re......

: _ret
i re......

: _eq
i eq......
i re......

: _neq
i ne......
i re......

: _lt
i lt......
i re......

: _gt
i gt......
i re......

: _fetch
i fe......
i re......

: _store
i st......
i re......

: _add
i ad......
i re......

: _sub
i su......
i re......

: _mul
i mu......
i re......

: _divmod
i di......
i re......

: _and
i an......
i re......

: _or
i or......
i re......

: _xor
i xo......
i re......

: _shift
i sh......
i re......

: _zret
i zr......
i re......
~~~

Though Nga allows for multiple instructions to be packed into a
single memory location (called a *cell*), Rx only packs a few
specific combinations.

Since calls and jumps take a value from the stack, a typical
call (in Muri assembly) would look like:

    i lica....
    r bye

Without packing this takes three cells: one for the lit, one
for the address, and one for the call. Packing drops it to two
since the lit/call combination can be fit into a single cell.
Likewise, I use a packed jump for use with quotations. These
saves several hundred cells (and thus fetch/decode cycles) when
loading the standard library.

The raw values for these are:

    2049  lica....
    1793  liju....

These are hardcoded in a few places later. I had previously
used a lookup, but this proved costly in processing time, so
hard coding proved better. (These places are clearly marked)

## Memory

Memory is a big, flat, linear array. The addressing starts at
zero and counts upwards towards a fixed upper limit (set by the
VM).

The basic memory accesses are handled via `fetch` and `store`.

The next two functions provide easier access to sequences of
data by fetching or storing a value and returning the next
address.

`fetch-next` takes an address and fetches the stored value. It
returns the next address and the stored value.

    :fetch-next  dup #1 + swap fetch ;

~~~
: fetch-next
i duliadsw
d 1
i fere....
~~~

`store-next` takes a value and an address. It stores the value
to the address and returns the next address.

    :store-next dup #1 + push store pop ;

~~~
: store-next
i duliadpu
d 1
i stpore..
~~~


## Conditionals

The Rx kernel provides three conditional forms:

    flag true-pointer false-pointer choose
    flag true-pointer   if
    flag false-pointer -if

`choose` is a conditional combinator which will execute one of
two functions, depending on the state of a flag. I use a little
hack here. I store the pointers into a jump table with two
fields, and use the flag as the index. Defaults to the *false*
entry, since a *true* flag is -1.

Note that this requires that the flags be -1 (for TRUE) and 0
(for FALSE). It's possible to make this more flexible, but at a
significant performance hit, so I'm leaving it this way.

    :choose
      &choice:false store &choice:true store
      &choice:false + fetch call ;

~~~
: choice:true
d 0

: choice:false
d 0

: choose
i listlist
r choice:false
r choice:true
i liadfeca
r choice:false
i re......
~~~

Next the two *if* forms. Note that `-if` falls into `if`. This
saves two cells of memory.

    :-if push #0 eq? pop \cc...... ;
    :if                  \cc...... ;

~~~
: -if
i pulieqpo
d 0
: if
i cc......
i re......
~~~

## Strings

The kernel needs two basic string operations for dictionary
searches: obtaining the length and comparing for equality.

Strings in Rx are zero terminated. This is a bit less elegant
than counted strings, but the implementation is quick and easy.

First up, string length. The process here is trivial:

* Make a copy of the starting point
* Fetch each character, comparing to zero

  * If zero, break the loop
  * Otherwise discard and repeat

* When done subtract the original address from the current one
* Then subtract one (to account for the zero terminator)

    :count repeat fetch-next 0; drop again ;
    :s:length dup count #1 - swap - ;

~~~
: count
i lica....
r fetch-next
i zr......
i drliju..
r count

: s:length
i dulica..
r count
i lisuswsu
d 1
i re......
~~~

String comparisons are harder. In high level code this is:

  dup fetch push n:inc swap
  dup fetch push n:inc pop dup pop
  -eq? [ drop-pair drop #0 pop pop drop drop ]
       [ 0; drop s:eq? pop pop drop drop ] choose drop-pair #-1 ;

I've rewritten this a few times. The current implementation is
fast enough, and not overly long. It may be worth looking into
a hash based comparsion in the future.

~~~
: mismatch
i drdrdrli
d 0
i popodrdr
i re......

: matched
i zr......
i drlica..
r s:eq
i popodrdr
i re......

: s:eq
i dufepuli
d 1
i adswdufe
i puliadpo
d 1
i duponeli
r mismatch
i lilica..
r matched
r choose
i drdrlire
d -1
~~~

## Interpreter & Compiler

### Compiler Core

The heart of the compiler is `comma` which stores a value into
memory and increments a variable (`Heap`) pointing to the next
free address.

    :, &Heap fetch store-next &Heap store ;

~~~
: comma
i lifelica
r Heap
r store-next
i listre..
r Heap
~~~

I also add a couple of additional forms. `comma:opcode` is used
to compile VM instructions into the current defintion. This is
where those functions starting with an underscore come into
play. Each wraps a single instruction. Using this I can avoid
hard coding the opcodes.

This performs a jump to the `comma` word instead of using a
`call/ret` to save a cell and slightly improve performance. I
will use this technique frequently.

    :comma:opcode  fetch , ;

~~~
: comma:opcode
i feliju..
r comma
~~~

`comma:string` is used to compile a string into the current
definition. As with `comma:opcode`, this uses a `jump` to
eliminate the final tail call.

    :($) fetch-next 0; , ($) ;
    :s,  ($) drop 0 , ;

~~~
: ($)
i lica....
r fetch-next
i zr......
i lica....
r comma
i liju....
r ($)

: comma:string
i lica....
r ($)
i drliliju
d 0
r comma
~~~

With the core functions above it's now possible to setup a few
more things that make compilation at runtime more practical.

First, a variable indicating whether we should compile or run a
function. In traditional Forth this would be **STATE**; I call
it `Compiler`.

This will be used by the *word classes*.

~~~
: Compiler
d 0
~~~

Next is *semicolon*; which compiles the code to terminate a
function and sets the `Compiler` to an off state (0). This
just needs to compile in a RET.

    :compiler:off #0 &Compiler store ;
    :; &_ret comma:opcode compiler:off ; immediate

~~~
: ;
i lilica..
r _ret
r comma:opcode
: compiler:off
i lilistre
d 0
r Compiler
~~~

### Word Classes

Rx is built over the concept of *word classes*. Word classes
are a way to group related words, based on their compilation
and execution behaviors. A *class handler* function is defined
to handle an execution token passed to it on the stack.

Rx provides several  classes with differing behaviors:

`class:data` provides for dealing with data structures.

| interpret            | compile                       |
| -------------------- | ----------------------------- |
| leave value on stack | compile value into definition |

    :class:data &Compiler fetch 0; drop &_lit comma:opcode comma ;

~~~
: class:data
i lifezr..
r Compiler
i drlilica
r _lit
r comma:opcode
i liju....
r comma
~~~

`class:word` handles most functions.

| interpret            | compile                       |
| -------------------- | ----------------------------- |
| call a function      | compile a call to a function  |

~~~
: class:word:interpret
i ju......

: class:word:compile
i lilica..
d 2049
r comma
i liju....
r comma

: class:word
i lifelili
r Compiler
r class:word:compile
r class:word:interpret
i liju....
r choose
~~~

`class:primitive` is a special class handler for functions that
correspond to Nga instructions.

| interpret            | compile                              |
| -------------------- | ------------------------------------ |
| call the function    | compile an instruction               |

~~~
: class:primitive
i lifelili
r Compiler
r comma:opcode
r class:word:interpret
i liju....
r choose
~~~

`class:macro` is the class handler for *compiler macros*. These
are functions that always get called. They can be used to
extend the language in interesting ways.

| interpret            | compile                       |
| -------------------- | ----------------------------- |
| call the function    | call the function             |

~~~
: class:macro
i ju......
~~~

The class mechanism is not limited to these classes. You can
write custom classes at any time. On entry the custom handler
should take the XT passed on the stack and do something with
it. Generally the handler should also check the `Compiler`
state to determine what to do in either interpretation or
compilation.

### Dictionary

Rx has a single dictionary consisting of a linked list of
headers. The current form of a header is shown in the chart
below.

| field | holds                              | accessor |
| ----- | ---------------------------------- | -------- |
| link  | link to the previous entry         | d:link   |
| xt    | link to start of the function      | d:xt     |
| class | link to the class handler function | d:class  |
| name  | zero terminated string             | d:name   |

The initial dictionary is constructed at the end of this file.
It'll take a form like this:

    : 0000
    d 0
    r _dup
    r class:primitive
    s dup

    : 0001
    r 0000
    r _drop
    r class:primitive
    s drop

    : 0002
    r 0001
    r _swap
    r class:primitive
    s swap

Each entry starts with a pointer to the prior entry (with a
pointer to zero marking the first entry in the dictionary), a
pointer to the start of the function, a pointer to the class
handler, and a null terminated string indicating the name
exposed to the Rx interpreter.

Rx stores the pointer to the most recent entry in a variable
called `Dictionary`. For simplicity, I just assign the last
entry an arbitrary label of 9999. This is set at the start of
the source. (See *In the Beginning...*)

Rx provides accessor functions for each field. Since the number
of fields (or their ordering) may change over time, using these
reduces the number of places where field offsets are hard coded.

~~~
: d:link
i re......

: d:xt
i liadre..
d 1

: d:class
i liadre..
d 2

: d:name
i liadre..
d 3
~~~

A traditional Forth has `create` to make a new dictionary entry
pointing to the next free location in `Heap`. Rx has `d:add-header`
which serves as a slightly more flexible base. You provide a
string for the name, a pointer to the class handler, and a
pointer to the start of the function. Rx does the rest.

In actual practice, I never use this outside of Rx. New words
are made using the `:` prefix, or `d:create` (once defined in
the standard library). At some point I may simplify this by
moving `d:create` into Rx and using it in place of `d:add-header`.

~~~
: d:add-header
i liju....
r _add-header
: _add-header
i lifepuli
r Heap
r Dictionary
i felica..
r comma
i lica....
r comma
i lica....
r comma
i lica....
r comma:string
i polistre
r Dictionary
~~~

Rx doesn't provide a traditional create as it's designed to
avoid assuming a normal input stream and prefers to take its
data from the stack.

### Dictionary Search

~~~
: Which
d 0

: Needle
d 0

: found
i listlire
r Which
r _nop

: find
i lilistli
d 0
r Which
r Dictionary
i fe......

: find_next
i zr......
i dulica..
r d:name
i lifelica
r Needle
r s:eq
i licc....
r found
i feliju..
r find_next

: d:lookup
i listlica
r Needle
r find
i lifere..
r Which
~~~

### Number Conversion

This code converts a zero terminated string into a number. The
approach is very simple:

* Store an internal multiplier value (-1 for negative, 1 for
  positive)
* Clear an internal accumulator value
* Loop:

  * Fetch the accumulator value
  * Multiply by 10
  * For each character, convert to a numeric value and add to
    the accumulator
  * Store the updated accumulator

* When done, take the accumulator value and the modifier and
  multiply them to get the final result

Rx only supports decimal numbers. If you want more bases, it's
pretty easy to add them later, but it's not needed in the base
kernel.

~~~
: next
i lica....
r fetch-next
i zr......
i lisuswpu
d 48
i swlimuad
d 10
i poliju..
r next

: check
i dufelieq
d 45
i zr......
i drswdrli
d -1
i swliadre
d 1

: s:to-number
i liswlica
d 1
r check
i liswlica
d 0
r next
i drmure..
~~~

### Token Processing

An input token has a form like:

    <prefix-char>string

Rx will check the first character to see if it matches a known
prefix. If it does, it will pass the string (sans prefix) to
the prefix handler. If not, it will attempt to find the token
in the dictionary.

Prefixes are handled by functions with specific naming
conventions. A prefix name should be:

    prefix:<prefix-char>

Where <prefix-char> is the character for the prefix. These are
compiler macros (using the `class:macro` class) and watch the
`Compiler` to decide how to deal with the token. To find a
prefix, Rx stores the prefix character into a string named
`prefixed`. It then searches for this string in the dictionary.
If found, it sets an internal variable (`prefix:handler`) to
the dictionary entry for the handler function. If not found,
`prefix:handler` is set to zero. The check, done by `prefix?`,
also returns a flag.

    '_         'prefix:no s:const
    'prefix:_  'prefixed  s:const

    'prefix:handler var

~~~
: prefix:no
d 32
d 0

: prefix:handler
d 0

: prefixed
s prefix:_
~~~

    :prefix:prepare
      fetch &prefixed #7 + store ;

~~~
: prefix:prepare
i feliliad
r prefixed
d 7
i stre....
~~~

    :prefix:has-token?
      dup s:length #1 0; drop-pair &prefix:no ;

~~~
: prefix:has-token?
i dulica..
r s:length
i lieqzr..
d 1
i drdrlire
r prefix:no
~~~

    :prefix?
      prefix:has-token? prefix:prepare &prefixed d:lookup
      dup &prefix:handler store #0 -eq? ;

~~~
: prefix?
i lica....
r prefix:has-token?
i lica....
r prefix:prepare
i lilica..
r prefixed
r d:lookup
i dulistli
r prefix:handler
d 0
i nere....
~~~

Rx makes extensive use of prefixes for implementing major parts
of the language, including  parsing numbers (prefix with `#`),
obtaining pointers (prefix with `&`), and defining functions
(using the `:` prefix).

| prefix | used for          | example |
| ------ | ----------------- | ------- |
| #      | numbers           | #100    |
| $      | ASCII characters  | $e      |
| &      | pointers          | &swap   |
| :      | definitions       | :foo    |
| (      | Comments          | (n-)    |

    :prefix:(  (s-)  drop ;

~~~
: prefix:(
i drre....
~~~

    :prefix:#  (s-n)  s:to-number class:data ;

~~~
: prefix:#
i lica....
r s:to-number
i liju....
r class:data
~~~

    :prefix:$  (s-c)  fetch class:data ;

~~~
: prefix:$
i feliju..
r class:data
~~~

    :prefix::  (s-)
      &class:word &Heap fetch d:add-header
      &Heap fetch &Dictionary d:xt store
      #-1 &Compiler store ;

~~~
: prefix::
i lilifeli
r class:word
r Heap
r d:add-header
i ca......
i lifelife
r Heap
r Dictionary
i lica....
r d:xt
i st......
: compiler:on
i lilistre
d -1
r Compiler
~~~

The `&` prefix is used to return the address of a named item.
This will correspond to the `d:xt` field of the word header.
In higher level Retro this would be:

    :prefix:&  (s-a)  d:lookup d:xt fetch class:data ;

As an example:

    #1 #2 &+ call        (call_`+`_via_pointer)
    &Heap fetch          (fetch_from_`Heap`)

In the latter case, the use of `class:data` means that I
don't *need* to use the `&` prefix, but I do this anyway as
I find it helps to provide a visual clue as to the intent of
the code.

This is also useful with combinators. If you are only using
a single word, using `&word` instead of `[ word ]` will be
smaller and faster.

For those familiar with traditional Forth, the `&` prefix
replaces both `'` and `[']`.

    :prefix:&  d:lookup d:xt fetch class:data ;

~~~
: prefix:&
i lica....
r d:lookup
i lica....
r d:xt
i feliju..
r class:data
~~~

### Quotations

Quotations are anonymous, nestable blocks of code. Rx uses them
for control structures and some aspects of data flow. A quote
takes a form like:

    [ #1 #2 ]
    #12 [ square #144 eq? [ #123 ] [ #456 ] choose ] call

Begin a quotation with `[` and end it with `]`. The code here
is slightly complicated by the fact that these have to be
nestable, and so must compile the appropriate jumps around
the nested blocks, in addition to properly setting and
restoring the `Compiler` state.

~~~
: [
i lifeliad
r Heap
d 2
i lifelica
r Compiler
r compiler:on
i lilica..
d 1793
r comma
i lifelili
r Heap
d 0
r comma
i ca......
i lifere..
r Heap

: ]
i lilica..
r _ret
r comma:opcode
i lifeswli
r Heap
r _lit
i lica....
r comma:opcode
i lica....
r comma
i swstlist
r Compiler
i lifezr..
r Compiler
i drdrre..
~~~

## Lightweight Control Structures

Rx provides a couple of functions for simple flow control apart
from using quotations. These are `repeat`, `again`, and `0;`.
An example of using them:

    :s:length
      dup [ repeat fetch-next 0; drop again ] call
      swap - #1 - ;

These can only be used within a definition or quotation. If you
need to use them interactively, wrap them in a quote and `call`
it.

~~~
: repeat
i lifere..
r Heap

: again
i lilica..
r _lit
r comma:opcode
i lica....
r comma
i liliju..
r _jump
r comma:opcode

: 0;
i liliju..
r _zret
r comma:opcode
~~~

I take a brief aside here to implement `push` and `pop`, which
move a value to/from the address stack. These are compiler
macros.

~~~
: push
i liliju..
r _push
r comma:opcode

: pop
i liliju..
r _pop
r comma:opcode
~~~

## Interpreter

The *interpreter* is what processes input. What it does is:

* Take a string
* See if the first character has a prefix handler

  * Yes: pass the rest of the string to the prefix handler
  * No: lookup in the dictionary

    * Found: pass xt of word to the class handler
    * Not found: report error via `err:notfound`

First, the handler for dealing with words that are not found.
This is defined here as a jump to the handler for the Nga *NOP*
instruction. It is intended that this be hooked into and changed.

As an example, in Rx code, assuming an I/O interface with some
support for strings and output:

    [ $? c:put sp 'word_not_found s:put ]
    &err:notfound #1 + store

An interface should either patch the jump, or catch it and do
something to report the error.

~~~
: err:notfound
i liju....
r _nop
i re......
~~~

`call:dt` takes a dictionary token and pushes the contents of
the `d:xt` field to the stack. It then calls the class handler
stored in `d:class`.

~~~
: call:dt
i dulica..
r d:xt
i feswlica
r d:class
i feju....
~~~

~~~
: input:source
d 0

: interpret:prefix
i ........
i ........
i lifezr..
r prefix:handler
i lifeliad
r input:source
d 1
i swliju..
r call:dt

: interpret:word
i lifeliju
r Which
r call:dt

: interpret:noprefix
i lifelica
r input:source
r d:lookup
i linelili
d 0
r interpret:word
r err:notfound
i liju....
r choose

: interpret
i liju....
r _interpret
: _interpret
i dulistli
r input:source
r prefix?
i ca......
i lililiju
r interpret:prefix
r interpret:noprefix
r choose
~~~

## Muri

Muri is my minimalist assembler for Nga. This is an attempt to
implement something similar in Retro.

This requires some knowledge of the Nga architecture to be
useful. The major elements are:

**Instruction Set**

Nga has 30 instructions. These are:

    0 nop   5 push  10 ret   15 fetch 20 div   25 zret
    1 lit   6 pop   11 eq    16 store 21 and   26 halt
    2 dup   7 jump  12 neq   17 add   22 or    27 ienum
    3 drop  8 call  13 lt    18 sub   23 xor   28 iquery
    4 swap  9 ccall 14 gt    19 mul   24 shift 29 iinvoke

The mnemonics allow for each name to be reduced to just two
characters. In the same order as above:

    0 ..    5 pu    10 re    15 fe    20 di    25 zr
    1 li    6 po    11 eq    16 st    21 an    26 ha
    2 du    7 ju    12 ne    17 ad    22 or    27 ie
    3 dr    8 ca    13 lt    18 su    23 xo    28 iq
    4 sw    9 cc    14 gt    19 mu    24 sh    29 ii

Up to four instructions can be packed into a single memory
location. (You can only use *no*p after a *ju*mp, *ca*ll,
*cc*all, *re*t, or *zr*et as these alter the instruction
pointer.)

So a bundled sequence like:

    lit 100
    lit 200
    add
    ret

Would look like:

    'liliadre i
    100       d
    200       d

And:

    lit s:eq?
    call

Would become:

    'lica.... i
    's:eq?    r

Note the use of `..` instead of `no` for the nop's; this is
done to improve readability a little.

Instruction bundles are specified as strings, and are converted
to actual instructions by the `i` word. As in the standard Muri
assembler, the RETRO version uses `d` for decimal values and `r`
for references to named functions.

The instructions table holds a hash value for each instruction
name. Lookups return the index of the instruction, which will
match the actual opcode. A value of zero marks the end of the
table.

Hashes are the two ASCII values multiplied together. E.g., for
`ha`, the hash is `#104 #97 *`.

This table can be regenerated using RETRO:

    { '.. 'li 'du 'dr 'sw 'pu 'po 'ju 'ca 'cc 're 'eq 'ne 'lt
      'gt 'fe 'st 'ad 'su 'mu 'di 'an 'or 'xo 'sh 'zr 'ha 'ie
      'iq 'ii }
    [ fetch-next swap fetch * 'd_ s:put n:put nl ] a:for-each

~~~
: Instructions
d 2116
d 11340
d 11700
d 11400
d 13685
d 13104
d 12432
d 12402
d 9603
d 9801
d 11514
d 11413
d 11110
d 12528
d 11948
d 10302
d 13340
d 9700
d 13455
d 12753
d 10500
d 10670
d 12654
d 13320
d 11960
d 13908
d 10088
d 10605
d 11865
d 11025
d 0
~~~

The `r` word resolves a reference and commas it into memory.

    :r d:lookup d:xt fetch , ;

~~~
: muri:r
i lica....
r d:lookup
i liadfe..
d 1
i liju....
r comma
~~~

The `i` word is used to assemble an instruction bundle and comma
it into memory. This is basically:

    :i
      dup get-opcode
      swap #2 + dup get-opcode
      swap #2 + dup get-opcode
      swap #2 + get-opcode
      #-24 shift swap #-16 shift +
      swap #-8 shift + + , ;

The actual implementation here is a bit different due to memory
constraints, but the above should make it fairly clear how this
works.

~~~
: muri:i
i dulica..
r get-opcode

i lica....
r (muri:i)
i lica....
r (muri:i)

i swliadli
d 2
r get-opcode
i ca......

i lishswli
d -24
d -16
i shadswli
d -8
i shadad..

i liju....
r comma
~~~

I use `name-to-id` to do a very crude hash of the instruction
names.

    :name-to-id dup fetch swap #1 + fetch * ;
    :name-to-id fetch-next swap fetch * ;

~~~
: name-to-id
i dufeswli
d 1
i adfemure
~~~

The `opcode-lookup` searches the `Instructions` table for a
hashed opcode name.

    :opcode-lookup
      repeat fetch-next 0; swap push over -eq? pop swap 0; drop again ;

~~~
: opcode-lookup
i lica....
r fetch-next
i zr......
i swpupudu
i poswnepo
i swzr....
i drliju..
r opcode-lookup
~~~

Inner bits of the `i` word, factored out to save space. The
interesting bit here is `get-opcode` which turns the returned
address in the `Instructions` table to an opcode number.

    :get-opcode
      name-to-id id-to-opcode &Instructions opcode-lookup
      nip &Instructions - #1 - ;

    :muri:i
      swap #2 + dup get-opcode ;

~~~
: (muri:i)
i swliaddu
d 2
: get-opcode
i lica....
r name-to-id
: id-to-opcode
i lilica....
r Instructions
r opcode-lookup
i swdrlisu
r Instructions
i lisure..
d 1
~~~

## The Initial Dictionary

This sets up the initial dictionary. Maintenance of this bit is
annoying, but it isn't necessary to change this unless you add
or remove new functions in the kernel.

~~~
: 0000
d 0
r _dup
r class:primitive
s dup
: 0001
r 0000
r _drop
r class:primitive
s drop
: 0002
r 0001
r _swap
r class:primitive
s swap
: 0003
r 0002
r _call
r class:primitive
s call
: 0004
r 0003
r _eq
r class:primitive
s eq?
: 0005
r 0004
r _neq
r class:primitive
s -eq?
: 0006
r 0005
r _lt
r class:primitive
s lt?
: 0007
r 0006
r _gt
r class:primitive
s gt?
: 0008
r 0007
r _fetch
r class:primitive
s fetch
: 0009
r 0008
r _store
r class:primitive
s store
: 0010
r 0009
r _add
r class:primitive
s +
: 0011
r 0010
r _sub
r class:primitive
s -
: 0012
r 0011
r _mul
r class:primitive
s *
: 0013
r 0012
r _divmod
r class:primitive
s /mod
: 0014
r 0013
r _and
r class:primitive
s and
: 0015
r 0014
r _or
r class:primitive
s or
: 0016
r 0015
r _xor
r class:primitive
s xor
: 0017
r 0016
r _shift
r class:primitive
s shift
: 0018
r 0017
r push
r class:macro
s push
: 0019
r 0018
r pop
r class:macro
s pop
: 0020
r 0019
r 0;
r class:macro
s 0;
: 0021
r 0020
r fetch-next
r class:word
s fetch-next
: 0022
r 0021
r store-next
r class:word
s store-next
: 0023
r 0022
r s:to-number
r class:word
s s:to-number
: 0024
r 0023
r s:eq
r class:word
s s:eq?
: 0025
r 0024
r s:length
r class:word
s s:length
: 0026
r 0025
r choose
r class:word
s choose
: 0027
r 0026
r if
r class:primitive
s if
: 0028
r 0027
r -if
r class:word
s -if
: 0029
r 0028
r prefix:(
r class:macro
s prefix:(
: 0030
r 0029
r Compiler
r class:data
s Compiler
: 0031
r 0030
r Heap
r class:data
s Heap
: 0032
r 0031
r comma
r class:word
s ,
: 0033
r 0032
r comma:string
r class:word
s s,
: 0034
r 0033
r ;
r class:macro
s ;
: 0035
r 0034
r [
r class:macro
s [
: 0036
r 0035
r ]
r class:macro
s ]
: 0037
r 0036
r Dictionary
r class:data
s Dictionary
: 0038
r 0037
r d:link
r class:word
s d:link
: 0039
r 0038
r d:xt
r class:word
s d:xt
: 0040
r 0039
r d:class
r class:word
s d:class
: 0041
r 0040
r d:name
r class:word
s d:name
: 0042
r 0041
r class:word
r class:word
s class:word
: 0043
r 0042
r class:macro
r class:word
s class:macro
: 0044
r 0043
r class:data
r class:word
s class:data
: 0045
r 0044
r d:add-header
r class:word
s d:add-header
: 0046
r 0045
r prefix:#
r class:macro
s prefix:#
: 0047
r 0046
r prefix::
r class:macro
s prefix::
: 0048
r 0047
r prefix:&
r class:macro
s prefix:&
: 0049
r 0048
r prefix:$
r class:macro
s prefix:$
: 0050
r 0049
r repeat
r class:macro
s repeat
: 0051
r 0050
r again
r class:macro
s again
: 0052
r 0051
r interpret
r class:word
s interpret
: 0053
r 0052
r d:lookup
r class:word
s d:lookup
: 0054
r 0053
r class:primitive
r class:word
s class:primitive
: 0055
r 0054
r Version
r class:data
s Version
: 0056
r 0055
r muri:i
r class:word
s i
: 0057
r 0056
r comma
r class:word
s d
: 0058
r 0057
r muri:r
r class:word
s r
: 9999
r 0058
r err:notfound
r class:word
s err:notfound
~~~

## Appendix: Words, Stack Effects, and Usage

| Word            | Stack     | Notes                                             |
| --------------- | --------- | ------------------------------------------------- |
| dup             | n-nn      | Duplicate the top item on the stack               |
| drop            | nx-n      | Discard the top item on the stack                 |
| swap            | nx-xn     | Switch the top two items on the stack             |
| call            | p-        | Call a function (via pointer)                     |
| eq?             | nn-f      | Compare two values for equality                   |
| -eq?            | nn-f      | Compare two values for inequality                 |
| lt?             | nn-f      | Compare two values for less than                  |
| gt?             | nn-f      | Compare two values for greater than               |
| fetch           | p-n       | Fetch a value stored at the pointer               |
| store           | np-       | Store a value into the address at pointer         |
| +               | nn-n      | Add two numbers                                   |
| -               | nn-n      | Subtract two numbers                              |
| *               | nn-n      | Multiply two numbers                              |
| /mod            | nn-mq     | Divide two numbers, return quotient and remainder |
| and             | nn-n      | Perform bitwise AND operation                     |
| or              | nn-n      | Perform bitwise OR operation                      |
| xor             | nn-n      | Perform bitwise XOR operation                     |
| shift           | nn-n      | Perform bitwise shift                             |
| fetch-next      | a-an      | Fetch a value and return next address             |
| store-next      | na-a      | Store a value to address and return next address  |
| push            | n-        | Move value from data stack to address stack       |
| pop             | -n        | Move value from address stack to data stack       |
| 0;              | n-n OR n- | Exit word (and `drop`) if TOS is zero             |
| s:to-number     | s-n       | Convert a string to a number                      |
| s:eq?           | ss-f      | Compare two strings for equality                  |
| s:length        | s-n       | Return length of string                           |
| choose          | fpp-?     | Execute *p1* if *f* is -1, or *p2* if *f* is 0    |
| if              | fp-?      | Execute *p* if flag *f* is true (-1)              |
| -if             | fp-?      | Execute *p* if flag *f* is false (0)              |
| Compiler        | -p        | Variable; holds compiler state                    |
| Heap            | -p        | Variable; points to next free memory address      |
| ,               | n-        | Compile a value into memory at `here`             |
| s,              | s-        | Compile a string into memory at `here`            |
| ;               | -         | End compilation and compile a *return* instruction|
| [               | -         | Begin a quotation                                 |
| ]               | -         | End a quotation                                   |
| Dictionary      | -p        | Variable; points to most recent header            |
| d:link          | p-p       | Given a DT, return the address of the link field  |
| d:xt            | p-p       | Given a DT, return the address of the xt field    |
| d:class         | p-p       | Given a DT, return the address of the class field |
| d:name          | p-p       | Given a DT, return the address of the name field  |
| class:word      | p-        | Class handler for standard functions              |
| class:primitive | p-        | Class handler for Nga primitives                  |
| class:macro     | p-        | Class handler for immediate functions             |
| class:data      | p-        | Class handler for data                            |
| d:add-header    | saa-      | Add an item to the dictionary                     |
| prefix:#        | s-        | # prefix for numbers                              |
| prefix::        | s-        | : prefix for definitions                          |
| prefix:&        | s-        | & prefix for pointers                             |
| prefix:$        | s-        | $ prefix for ASCII characters                     |
| prefix:(        | s-        | ( prefix for comments                             |
| repeat          | -a        | Start an unconditional loop                       |
| again           | a-        | End an unconditional loop                         |
| interpret       | s-?       | Evaluate a token                                  |
| d:lookup        | s-p       | Given a string, return the DT (or 0 if undefined) |
| Version         | -a        | Variable; holds a version identifier              |
| err:notfound    | -         | Handler for token not found errors                |

## Legalities

Rx is Copyright (c) 2016-2020, Charles Childers

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice
appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

My thanks go out to Michal J Wallace, Luke Parrish, JGL, Marc
Simpson, Oleksandr Kozachuk, Jay Skeer, Greg Copeland, Aleksej
Saushev, Foucist, Erturk Kocalar, Kenneth Keating, Ashley
Feniello, Peter Salvi, Christian Kellermann, Jorge Acereda,
Remy Moueza, John M Harrison, and Todd Thomas.

All of these great people helped in the development of RETRO 10
and 11, without which Rx wouldn't have been possible.
