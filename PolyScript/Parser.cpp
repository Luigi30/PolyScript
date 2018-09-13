#include "stdafx.h"
#include "Parser.h"

namespace PolyScript
{
	namespace Parser
	{
		int peek()
		{
			int c = getchar();
			ungetc(c, stdin);
			return c;
		}

		// Skips the input until newline is found. Newline is one of \r, \r\n or \n.
		void skip_line(void) {
			for (;;) {
				int c = getchar();
				if (c == EOF || c == '\n')
					return;
				if (c == '\r') {
					if (peek() == '\n')
						getchar();
					return;
				}
			}
		}

		int read_number(int val) {
			while (isdigit(peek()))
				val = val * 10 + (getchar() - '0');
			return val;
		}

		Object * read_numeric_string(char c)
		{
			// parse the string and figure out if it's a float or an int.
			char buf[NUMBER_MAX_LEN + 1];
			int len = 1;
			buf[0] = c;

			bool decimal_flag = false;
			bool negative_flag = false;

			while (isdigit(peek()) || peek() == '.' || peek() == '-')
			{
				if (len >= NUMBER_MAX_LEN)
					error("Number too long");

				if (peek() == '.' && decimal_flag == true)
					error("Invalid numeric value: two decimal points");
				else if (peek() == '.')
					decimal_flag = true;

				if (peek() == '-' && negative_flag == true)
					error("Invalid numeric value: two negative signs");
				else if (peek() == '-')
					negative_flag = true;

				buf[len++] = getchar();
			}

			if (decimal_flag)
				return Object::MakeFloat(atof(buf));
			else
				return Object::MakeInt(atoi(buf));
		}

		Object *read_symbol(char c) {
			char buf[SYMBOL_MAX_LEN + 1];
			int len = 1;
			buf[0] = c;
			while (isalnum(peek()) || peek() == '-') {
				if (SYMBOL_MAX_LEN <= len)
					error("Symbol name too long");
				buf[len++] = getchar();
			}
			buf[len] = '\0';
			return Object::intern(buf);
		}

		// Reader macro ' (single quote).
		// It reads an expression and returns (quote <expr>).
		Object *read_quote(void) {
			Object *sym = Object::intern("quote");
			return Object::cons(sym, Object::cons(read(), Nil));
		}

		// Reads a list. Note that '(' has already been read.
		Object *read_list(void) {
			Object *obj = read();
			if (!obj)
				error("Unclosed parenthesis");
			if (obj == Dot)
				error("Stray dot");
			if (obj == Cparen)
				return Nil;
			Object *head, *tail;
			head = tail = Object::cons(obj, Nil);

			for (;;) {
				Object *obj = read();
				if (!obj)
					error("Unclosed parenthesis");
				if (obj == Cparen)
					return head;
				if (obj == Dot) {
					tail->cdr = read();
					if (read() != Cparen)
						error("Closed parenthesis expected after dot");
					return head;
				}
				tail->cdr = Object::cons(obj, Nil);
				tail = tail->cdr;
			}
		}

		// Reads a line from the console.
		Object *read(void) {
			for (;;) {
				int c = getchar();
				if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
					continue;
				if (c == EOF)
					return NULL;
				if (c == ';') {
					skip_line();
					continue;
				}
				if (c == '(')
					return read_list();
				if (c == ')')
					return Cparen;
				if (c == '.')
					return Dot;
				if (c == '\'')
					return read_quote();
				if (isdigit(c) || c == '-')
					return read_numeric_string(c);
				if (isalpha(c) || strchr("+=!@#$%^&*", c))
					return read_symbol(c);
				error("Don't know how to handle %c", c);
			}
		}

		// Prints the given object.
		void print(Object *obj) {
			switch (obj->tag) {

			case T_ATOM:
				switch (obj->atom_subtype)
				{
				case AT_INT:
					printf("%d", obj->value);
					return;
				case AT_FLOAT:
					printf("%f", obj->float_value);
					return;
				case AT_SYMBOL:
					printf("%s", obj->name);
					return;
				}

			case T_CELL:
				printf("(");
				for (;;) {
					print(obj->car);
					if (obj->cdr == Nil)
						break;
					if (obj->cdr->tag != T_CELL) {
						printf(" . ");
						print(obj->cdr);
						break;
					}
					printf(" ");
					obj = obj->cdr;
				}
				printf(")");
				return;
				/*
			case T_SYMBOL:
				printf("%s", obj->name);
				return;
				*/
			case T_PRIMITIVE:
				printf("<primitive>");
				return;
			case T_FUNCTION:
				printf("<function>");
				return;
			case T_MACRO:
				printf("<macro>");
				return;
			case T_SPECIAL:
				if (obj == Nil)
					printf("()");
				else if (obj == True)
					printf("t");
				else
					error("Bug: print: Unknown subtype: %d", obj->subtype);
				return;
			default:
				error("Bug: print: Unknown tag type: %d", obj->tag);
			}
		}
	}
}