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

std::wstring my_html = L"<h1>Hello, World!"</h1>";
std::wstring markdown = ConvertHTMLToMarkdown(my_html);
```

This will return a string containing the markdown equivalent of the input HTML.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.

## Author
Authored by Erdit (azulx), please feel free to reach out or contribute to this project!
