/*
 * A Simple HTML to Markdown Parser
 *
 * Copyright (c) 2023 Erdit Llugiqi (https://github.com/azulx)
 *
 * Licensed under the MIT License.
 */

#ifndef HTML_TO_MD_PARSER_HPP_
#define HTML_TO_MD_PARSER_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <unordered_map>
#include <map>

// Basic HTML tags that we can convert easily
std::unordered_map<std::wstring, std::pair<std::wstring, std::wstring>> basic_markdown_tags = {
	{L"p", {L"\n\n", L""}},
	{L"strong", {L"**", L"**"}},
	{L"b", {L"**", L"**"}},
	{L"em", {L"_", L"_"}},
	{L"i", {L"_", L"_"}},
	{L"del", {L"~~", L"~~"}},
	{L"ins", {L"__", L"__"}},
	{L"br", {L"\n", L""}},
	{L"hr", {L"\n\n_________________\n\n", L""}},
	{L"form", {L"\n\n[form]\n\n", L""}},
	{L"blockquote", {L"\n> ", L""}},
};

// HTML tags to be entirely skipped over and removed (must be a generic styled tag (i.e. <script></script>))
const std::vector<std::wstring> skip_tags = { L"script", L"style", L"title" };

// HTML entities and their equivalent
const std::unordered_map<std::wstring, std::wstring> html_entities = {
	{L"&quot;", L"\""},
	{L"&apos;", L"'"},
	{L"&amp;", L"&"},
	{L"&lt;", L"<"},
	{L"&nbsp;", L" "},
	{L"&gt;", L">"}
};

// Tags that "self-close", they don't have a closing tag (<br> doesn't have a </br> in normal usage)
const std::set<std::wstring> self_closing_tags = { L"area", L"base", L"br", L"col", L"command", L"embed", L"hr", L"img", L"input",
	L"keygen", L"link", L"meta", L"param", L"source", L"track", L"wbr" };

enum ASTNodeType
{
	ROOT,       // e.g. <html>
	TEXT,       // e.g. <p>Hello, World</p> - "Hello, World" = TEXT Node.
	ELEMENT,    // e.g. <div>
};

struct ASTNode
{
	ASTNodeType type;  // The type of the node (root, element, or text)
	std::wstring name;  // The name of the element node (such as "div", "p", "a", etc.)
	std::wstring value; // The value of the text node (such as "Hello, World!", "this is text", etc.)

	std::map<std::wstring, std::wstring> attributes; // HTML attributes, take <img src="image.jpg" alt="An image"> - "src" and "alt" are attributes

	// The child nodes of this node
	// Say you had <h1>Header</h1>, the child is the text node "Header"
	std::vector<ASTNode> children;
};


/*
* @brief Removes leading and trailing whitespace from a given string, which is useful in parsing HTML tags and attributes.
*
* @param str The string to trim whitespaces from
* @returns The trimmed string
*/
std::wstring Trim(const std::wstring& str)
{
	std::size_t first = str.find_first_not_of(L" \t\n\r");
	if (first == std::wstring::npos)
	{
		return L"";
	}
	std::size_t last = str.find_last_not_of(L" \t\n\r");
	return str.substr(first, last - first + 1);
}

/**
 * @brief Parse the tag from a string.
 *
 * This function takes a string as input, which contains the tag name and optionally some attributes.
 * The function splits the string into the tag name and the rest of the string, which contains the attributes.
 *
 * @param tag The input string containing the tag name and attributes.
 * @return A pair of wide strings, where the first string is the tag name and the second string is the attributes.
 */
std::pair<std::wstring, std::wstring> ParseTag(const std::wstring& tag) {
	std::size_t space_index = tag.find(' ');
	std::wstring tag_name = space_index == std::wstring::npos ? tag : tag.substr(0, space_index);
	std::wstring attributes = space_index == std::wstring::npos ? L"" : Trim(tag.substr(space_index + 1));

	return { tag_name, attributes };
}

/**
 * @brief Parse the attributes from a tag.
 *
 * This function takes a string as input, which contains the attributes of a tag.
 * Each attribute is of the form key=value, and the function builds a map from keys to values.
 *
 * @param attributes The input string containing the attributes.
 * @return A map from keys to values, where both keys and values are wide strings. If a value is enclosed in double quotes, the quotes are removed.
 */std::map<std::wstring, std::wstring> ParseAttributes(const std::wstring& attributes) {
	 std::map<std::wstring, std::wstring> attribute_map;

	 if (!attributes.empty()) {
		 std::wistringstream attribute_stream(attributes);
		 std::wstring attribute;

		 while (attribute_stream >> attribute) {
			 std::size_t equal_index = attribute.find('=');

			 if (equal_index != std::wstring::npos) {
				 std::wstring key = attribute.substr(0, equal_index);
				 std::wstring value = attribute.substr(equal_index + 1);

				 if (value.length() >= 2 && value[0] == '"' && value[value.length() - 1] == '"') {
					 value = value.substr(1, value.length() - 2);
				 }

				 attribute_map[key] = value;
			 }
		 }
	 }

	 return attribute_map;
 }


 /*
  * @brief Parses an HTML string into an abstract syntax tree (AST).
  *
  * The AST represents the structure of the HTML, with each node in the tree
  * representing either an HTML element (with its tag name and attributes) or a
  * text node (with its textual content).
  *
  * This function uses a stack to keep track of the current node in the tree,
  * and adds new nodes to the tree as it encounters more HTML tags and text nodes
  * in the input string.
  *
  * @param The HTML string to parse
  * @return The root node of the resulting AST
  */
 ASTNode ParseHTMLToAST(const std::wstring& html_input) {
	 ASTNode root;
	 root.type = ROOT;
	 std::vector<ASTNode*> stack;
	 stack.push_back(&root);

	 for (int current_index = 0; current_index < html_input.length();) {
		 char c = html_input[current_index];
		 if (c == '<') {
			 std::size_t end_tag_index = html_input.find('>', current_index);

			 if (end_tag_index == std::wstring::npos) {
				 //TODO: maybe ignore and still create markdown without this specific incomplete part?
				 throw std::out_of_range("malformed or incomplete HTML input (no closing '>')!");
			 }

			 std::wstring tag = html_input.substr(current_index + 1, end_tag_index - current_index - 1);

			 if (tag.substr(0, 3) == L"!--") { //special case, doesn't have a closing/opening "<"/">".
				 std::size_t end_comment_index = html_input.find(L"-->", current_index);
				 if (end_comment_index == std::wstring::npos)
					 throw std::out_of_range("malformed or incomplete HTML input (no closing '-->')!");
				 current_index = end_comment_index + 3;
			 }

			 else if (std::find(skip_tags.begin(), skip_tags.end(), tag) != skip_tags.end()) {
				 std::size_t end_tag_index = html_input.find(L"</" + tag + L">", current_index);
				 if (end_tag_index == std::wstring::npos)
					 throw std::out_of_range("malformed or incomplete HTML input (no closing tag!)!");
				 current_index = end_tag_index + tag.size() + 3;
			 }

			 else {
				 bool is_closing_tag = false;

				 if (tag[0] == '/') {
					 is_closing_tag = true;
					 tag = tag.substr(1);
				 }

				 auto [tag_name, attributes] = ParseTag(tag);

				 ASTNode node;
				 node.type = ELEMENT;
				 node.name = tag_name;

				 if (!attributes.empty() && !is_closing_tag) {
					 node.attributes = ParseAttributes(attributes);
				 }

				 if (!is_closing_tag) {
					 stack.back()->children.push_back(node);
					 if (self_closing_tags.find(tag_name) == self_closing_tags.end()) {
						 stack.push_back(&stack.back()->children.back());
					 }
				 }

				 else {
					 if (stack.back()->name != tag_name)
						 throw std::runtime_error("closing tag does not match with corresponding opening tag!");
					 stack.pop_back();
				 }

				 current_index = end_tag_index + 1;
			 }
		 }

		 else {
			 std::size_t end_text_index = html_input.find('<', current_index);
			 if (end_text_index == std::wstring::npos)
				 end_text_index = html_input.length();

			 std::wstring text = html_input.substr(current_index, end_text_index - current_index);

			 for (const auto& entity : html_entities) {
				 std::size_t pos = 0;
				 while ((pos = text.find(entity.first, pos)) != std::wstring::npos) {

					 text.replace(pos, entity.first.length(), entity.second);
					 pos += entity.second.length();
				 }
			 }

			 ASTNode node;
			 node.type = TEXT;
			 node.value = text;
			 stack.back()->children.push_back(node);
			 current_index = end_text_index;
		 }
	 }

	 return root;
 }

 /*
 * @brief Converts ASTNode to markdown wstring.
 *
 * This function takes an ASTNode representing an HTML document and recursively
 * converts it into a Markdown string representation.
 *
 * @param node The ASTNode, the root node of the AST.
 * @return The markdown version of the HTML.
 */
 std::wstring AstToMarkdown(const ASTNode& node, int list_level = 0)
 {
	 std::wstring markdown;
	 std::wstring list_prefix(list_level, L'\t');
	 static int counter = 1;
	 std::vector<int> counters(1, 1);

	 for (auto& child : node.children)
	 {
		 switch (child.type)
		 {
		 case ROOT:
			 markdown += AstToMarkdown(child);
			 break;
		 case TEXT:
			 markdown += child.value;
			 break;
		 case ELEMENT:
			 if (child.attributes.find(L"class") != child.attributes.end() && child.attributes.find(L"class")->second == L"hidden") {
				 break;
			 }

			 if (basic_markdown_tags.find(child.name) != basic_markdown_tags.end()) {
				 markdown += basic_markdown_tags[child.name].first + AstToMarkdown(child) + basic_markdown_tags[child.name].second;
				 if (child.name == L"p" || child.name == L"hr" || child.name.substr(0, 1) == L"h") {
					 markdown += L"\n";  // Add newline after paragraph, horizontal rule, or header
				 }
				 break;
			 }

			 if (child.name == L"a")
			 {
				 std::wstring href = child.attributes.at(L"href");
				 std::wstring inner_text = AstToMarkdown(child);
				 if (inner_text.empty())
				 {
					 markdown += href;
				 }
				 else
				 {
					 markdown += L"[" + inner_text + L"](" + href + L")";
				 }
			 }

			 else if (child.name == L"li")
			 {
				 if (node.name == L"ol")
				 {
					 markdown += L"\n" + list_prefix + std::to_wstring(counters.back()++) + L". " + AstToMarkdown(child);
				 }

				 else
				 {
					 markdown += L"\n" + list_prefix + L"- " + AstToMarkdown(child);
				 }
			 }

			 else if (child.name == L"ol" || child.name == L"ul")
			 {
				 counters.push_back(1);
				 list_level++;
				 markdown += L"\n" + AstToMarkdown(child);
				 list_level--;
				 counters.pop_back();
			 }

			 else if (child.name == L"img")
			 {
				 std::wstring src = child.attributes.at(L"src");
				 std::wstring alt = child.attributes.at(L"alt");
				 markdown += L"![" + alt + L"](" + src + L")\n";
			 }

			 else if (child.name == L"code")
			 {
				 if (!node.name.empty() && node.name != L"pre")
				 {
					 markdown += L"```" + AstToMarkdown(child) + L"```\n";  // Add newline after code block
				 }
				 else
				 {
					 markdown += AstToMarkdown(child);
				 }
			 }

			 else if (child.name.substr(0, 1) == L"h" && child.name.size() == 2)
			 {
				 int header_level = std::stoi(child.name.substr(1, 1));
				 std::wstring header_str(header_level, L'#');
				 markdown += L"\n" + header_str + L" " + AstToMarkdown(child) + L"\n";  // Add newline before and after header
			 }

			 else if (child.name == L"input")
			 {
				 std::wstring type = child.attributes.at(L"type");
				 markdown += L"\n\n[input: " + type + L"]\n\n";
			 }
			 else if (child.name == L"label")
			 {
				 std::wstring value = child.attributes.at(L"value");
				 markdown += L"\n\n[label: " + value + L"]\n\n";
			 }

			 else if (child.name == L"table")
			 {
				 std::wstring header_row;
				 std::wstring separator_row;
				 std::wstring body_rows;
				 std::wstring footer_rows;
				 std::wstring caption_text;

				 for (const ASTNode& table_child : child.children)
				 {
					 if (table_child.name == L"tr")
					 {
						 std::wstring row_text;
						 for (const ASTNode& cell : table_child.children)
						 {
							 if (cell.name == L"td" || cell.name == L"th")
							 {
								 row_text += L"|" + AstToMarkdown(cell);
							 }
						 }
						 row_text += L"|\n";
						 body_rows += row_text;
					 }

					 else if (table_child.name == L"thead" || table_child.name == L"tbody")
					 {
						 for (const ASTNode& row : table_child.children)
						 {
							 if (row.name == L"tr")
							 {
								 std::wstring row_text;
								 for (const ASTNode& cell : row.children)
								 {
									 if (cell.name == L"td" || cell.name == L"th")
									 {
										 row_text += L"|" + AstToMarkdown(cell);
										 if (table_child.name == L"thead")
										 {
											 separator_row += L"|---";
										 }
									 }
								 }
								 row_text += L"|\n";
								 if (table_child.name == L"thead")
								 {
									 separator_row += L"|\n";
									 header_row += row_text;
								 }
								 else
								 {
									 body_rows += row_text;
								 }
							 }
						 }
					 }

					 else if (table_child.name == L"tfoot")
					 {
						 for (const ASTNode& row : table_child.children)
						 {
							 if (row.name == L"tr")
							 {
								 std::wstring row_text;
								 for (const ASTNode& cell : row.children)
								 {
									 if (cell.name == L"td" || cell.name == L"th")
									 {
										 row_text += L"|" + AstToMarkdown(cell);
									 }
								 }
								 row_text += L"|\n";
								 footer_rows += row_text;
							 }
						 }
					 }

					 else if (table_child.name == L"caption")
					 {
						 caption_text = L"\n**" + AstToMarkdown(table_child) + L"**\n";
					 }
				 }

				 markdown += caption_text + header_row + separator_row + body_rows + footer_rows;
			 }

			 else
				 markdown += AstToMarkdown(child);

			 break;
		 default:
			 break;
		 }
	 }

	 return markdown;
 }

 std::wstring ConvertHTMLToMarkdown(const std::wstring& html_input)
 {
	 const std::set<std::wstring> tags = { L"<html", L"<head", L"<body", L"<div", L"<p", L"<a", L"<img", L"<span", L"<table", L"<tr", L"<td", L"<ul", L"<li", L"<h1", L"<h2", L"<h3", L"<h4", L"<h5", L"<h6", L"</html", L"</head", L"</body", L"</div", L"</p", L"</a", L"</img", L"</span", L"</table", L"</tr", L"</td", L"</ul", L"</li", L"</h1", L"</h2", L"</h3", L"</h4", L"</h5", L"</h6" };

	 for (const auto& tag : tags)
	 {
		 if (html_input.find(tag) != std::wstring::npos)
		 {
			 try
			 {
				 ASTNode ast = ParseHTMLToAST(html_input);
				 return AstToMarkdown(ast);
			 }

			 catch (const std::exception& e)
			 {
				 // If there's an error, it's likely that the input bypassed the HTML check above.
				 // Therefore, we can assume it's either not HTML or it's malformed.

				 // std::wcout << "Exception: " << e.what() << std::endl;

				 return html_input;
			 }
		 }
	 }

	 return html_input;
 }


#endif  // HTML_TO_MD_PARSER_HPP_