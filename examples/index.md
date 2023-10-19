# website generation with markdown-like syntax and shit 😈
##### (markdown spec prepare to be ignored!)
###### (if you do anything I find stupid that is)

__underlined__ (this one is not commonmark, but it makes more sense to me) and **bold** and ~~strikethrough~~ and direct newlines\
_italics_ and *italics (again)* and ^superscript^ and ~subscript~ and `inline code`

aside from those there's also block structures!

> we can quote stuff
> hell yea
> let's go

### and also some other stuff

- like a UL
- that's fun

1) or an ordered list
2) yeag
3) with nesting
  - uwu
  - owo
  - awa
  - nya
    1. more nesting
    2. let us fucking go

```
codeblocks (which for now have no syntax highlighting yet)
```

~~~
and a different codefence
~~~

| table time | is it? |
|------------|--------|
| hell yea table time | absolutely |
| gods this table looks like shit | uwu |
| hopefully it looks better when it's actually rendering | meow |

### but it's not just 'vanilla' markdown

%embed ./embed.md

as well as direct links to other [markdown files](%./linked.md), which cause those to be compiled at the same time as well
