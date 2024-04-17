# HTML to Markdown Parser

The HTML to Markdown Parser is a simplistic, single-file tool for converting HTML code into Markdown. It's written in C++ and is designed to handle a wide range of HTML tags and structures.

## Features
* Tested and functional from C++14 and up
* Conversion of most common HTML tags to their Markdown equivalent.
* Handling of nested lists and table structures.
* Error handling for malformed or incomplete HTML input.
* Single-file, easy implementation to any project
* No regex, all simplistic string manipulation

## Usage
Include the `HTMLtoMD.hpp` file in your project and call the `ConvertHTMLToMarkdown` function with your HTML string as the argument:

```cpp
#include "HTMLtoMD.hpp"

std::wstring my_html = L"<h1>Hello, World!</h1>";
std::wstring markdown = html_to_md::ConvertHTMLToMarkdown(my_html);
```

This will return a string containing the markdown equivalent of the input HTML.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.

## How it Works
The HTML to Markdown Parser operates in two main steps: parsing the HTML into an Abstract Syntax Tree (AST) and then converting the AST into Markdown.

### Parsing HTML to AST
In the first step, the parser reads through the input HTML and constructs an AST. Each node in the AST represents an HTML element (such as a paragraph, link, or header), a text block, or the root of the document.

Each element node in the AST holds information about the element's name (like "p" for paragraph, "a" for link, etc.), its attributes (if any), and a list of its child nodes. Text nodes hold the text content. This structure allows the parser to capture the hierarchy and nesting of the HTML elements.

### Converting AST to Markdown
In the second step, the parser walks through the AST and converts each node into its corresponding Markdown syntax. For example, it converts a paragraph node (`<p>...</p>`) into text enclosed by two newline characters, a header node (`<h1>...</h1>`) into text prefixed by a `'#'` character (or a `<h2>` with a `##`... etc.), and so on.

Nested structures like lists and tables are handled via a recursive approach: when the parser encounters a node that can contain other nodes (like a list or a table), it makes a recursive call to process the child nodes associated with it.

The parser also manages the indentation level of nested lists by using a counter, which increments when a new list is encountered and decrements when a list ends. This counter is used to prefix the right number of tab characters to the list items.

