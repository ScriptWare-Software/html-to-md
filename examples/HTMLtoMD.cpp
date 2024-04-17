#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <locale>
#include "../HTMLtoMD.hpp"

int main() {
    std::wifstream input_file("example1.html");
    std::wofstream output_file("example1.md");

    if (!input_file.is_open()) {
        std::cout << "Failed to open input file." << std::endl;
        return 1;
    }

    if (!output_file.is_open()) {
        std::cout << "Failed to open output file." << std::endl;
        return 1;
    }

    input_file.imbue(std::locale(input_file.getloc(), new std::codecvt_utf8<wchar_t>));
    output_file.imbue(std::locale(output_file.getloc(), new std::codecvt_utf8<wchar_t>));
    
    std::wstring html_content((std::istreambuf_iterator<wchar_t>(input_file)),
                               std::istreambuf_iterator<wchar_t>());

    std::wstring markdown_content = html_to_md::ConvertHTMLToMarkdown(html_content);

    output_file << markdown_content;

    input_file.close();
    output_file.close();

    std::wcout << L"Conversion completed. Markdown content saved to output.md" << std::endl;

    return 0;
}