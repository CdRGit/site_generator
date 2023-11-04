#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "sitegen/markless/document.h"

static void free_node(markless_component* component);

static void free_nodes(vector(markless_component*) components) {
	for (int i = 0; i < vector_count(components); i++) {
		free_node(components[i]);
	}

	vector_free(components);
}

static void free_node(markless_component* component) {
	_Static_assert(ML_CT_COUNT == 13, "non-exhaustive");
	switch (component->type) {
		case ML_CT_ROOT_DOCUMENT: {
			free_nodes(component->root->children);
			free(component->root);
		} break;
		case ML_CT_HEADER: {
			free_nodes(component->header->children);
			free(component->header);
		} break;
		case ML_CT_PARAGRAPH: {
			free_nodes(component->paragraph->children);
			free(component->paragraph);
		} break;
		case ML_CT_BLOCKQUOTE_BODY: {
			free_nodes(component->blockquote_body->children);
			free(component->blockquote_body);
		} break;
		case ML_CT_BLOCKQUOTE_HEADER: {
			free_nodes(component->blockquote_header->children);
			free(component->blockquote_header);
		} break;
		case ML_CT_ORDERED_LIST: {
			free_nodes(component->ordered_list->items);
			free(component->ordered_list);
		} break;
		case ML_CT_ORDERED_LIST_ITEM: {
			free_nodes(component->ordered_list_item->children);
			free(component->ordered_list_item);
		} break;
		case ML_CT_UNORDERED_LIST: {
			free_nodes(component->unordered_list->items);
			free(component->unordered_list);
		} break;
		case ML_CT_UNORDERED_LIST_ITEM: {
			free_nodes(component->unordered_list_item->children);
			free(component->unordered_list_item);
		} break;
		case ML_CT_CODEBLOCK: {
			vector_free(component->codeblock->language);
			vector_free(component->codeblock->options);
			vector_free(component->codeblock->text);
			free(component->codeblock);
		} break;
		case ML_CT_TEXT: {
			vector_free(component->text->text);
			free(component->text);
		} break;
		case ML_CT_NEWLINE: {} break;
		case ML_CT_HORIZONTAL_RULE: {} break;
		case ML_CT_COUNT:
			fprintf(stderr, "unreachable\n");
			exit(1);
	}
	free(component);
}

void free_document(markless_doc* document) {
	free_nodes(document->children);
	free(document);
}

static void print_node(markless_component* component, int depth);

static void print_nodes(vector(markless_component*) components, int depth) {
	for (int i = 0; i < vector_count(components); i++) {
		print_node(components[i], depth);
	}
}

static void print_indent(int depth) {
	for (int i = 0; i < depth; i++) printf("    ");
}

static void print_stringview(stringview view, int depth) {
	print_indent(depth);
	for (int i = 0; i < view.len; i++) {
		char c = view.data[i];
		if (c == '\n') {
			printf("\n");
			print_indent(depth);
		}
		else printf("%c", c);
	}
}

static void print_string(char* string, int depth) {
	stringview view;
	view.data = string;
	view.len = strlen(string);
	print_stringview(view, depth);
}

static void print_text(vector(char) text, int depth) {
	stringview view;
	view.data = text;
	view.len = (size_t)vector_count(text);
	print_stringview(view, depth);
}

static void print_node(markless_component* component, int depth) {
	_Static_assert(ML_CT_COUNT == 13, "non-exhaustive");
	switch (component->type) {
		case ML_CT_ROOT_DOCUMENT: {
			print_string("ROOT", depth);
			printf("\n");
			print_nodes(component->root->children, depth+1);
		} break;
		case ML_CT_HEADER: {
			print_string("HEADER(", depth);
			printf("%d)", component->header->level);
			printf("\n");
			print_nodes(component->header->children, depth+1);
		} break;
		case ML_CT_PARAGRAPH: {
			print_string("PARAGRAPH", depth);
			printf("\n");
			print_nodes(component->paragraph->children, depth+1);
		} break;
		case ML_CT_BLOCKQUOTE_BODY: {
			print_string("BLOCKQUOTE(BODY)", depth);
			printf("\n");
			print_nodes(component->blockquote_body->children, depth+1);
		} break;
		case ML_CT_BLOCKQUOTE_HEADER: {
			print_string("BLOCKQUOTE(HEADER)", depth);
			printf("\n");
			print_nodes(component->blockquote_header->children, depth+1);
		} break;
		case ML_CT_ORDERED_LIST: {
			print_string("ORDERED_LIST", depth);
			printf("\n");
			print_nodes(component->ordered_list->items, depth+1);
		} break;
		case ML_CT_ORDERED_LIST_ITEM: {
			print_string("ORDERED_LIST_ITEM(", depth);
			printf("%d)", component->ordered_list_item->number);
			printf("\n");
			print_nodes(component->ordered_list_item->children, depth+1);
		} break;
		case ML_CT_UNORDERED_LIST: {
			print_string("UNORDERED_LIST", depth);
			printf("\n");
			print_nodes(component->unordered_list->items, depth+1);
		} break;
		case ML_CT_UNORDERED_LIST_ITEM: {
			print_string("UNORDERED_LIST_ITEM", depth);
			printf("\n");
			print_nodes(component->unordered_list_item->children, depth+1);
		} break;
		case ML_CT_CODEBLOCK: {
			print_indent(depth);
			printf("CODEBLOCK('%.*s', '%.*s')\n", (int)vector_count(component->codeblock->language), component->codeblock->language, (int)vector_count(component->codeblock->options), component->codeblock->options);
			print_text(component->codeblock->text, depth+1);
			printf("\n");
		} break;
		case ML_CT_TEXT: {
			print_string("TEXT", depth);
			printf("\n");
			print_text(component->text->text, depth+1);
			printf("\n");
		} break;
		case ML_CT_NEWLINE: {
			print_string("<NEWLINE>", depth);
			printf("\n");
		} break;
		case ML_CT_HORIZONTAL_RULE: {
			print_string("<HORIZONTAL_RULE>", depth);
			printf("\n");
		} break;
		case ML_CT_COUNT:
			fprintf(stderr, "unreachable\n");
			exit(1);
	}
}

void print_document(markless_doc* document) {
	print_nodes(document->children, 0);
}
