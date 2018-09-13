#pragma once

#include "PolyScript.h"

#include <cstdlib>
#include <string>

#include <Windows.h>

namespace PolyScript
{
	namespace Parser
	{
#define SYMBOL_MAX_LEN 200
#define NUMBER_MAX_LEN 100

		int peek();
		void skip_line(void);

		Object * read_string_input(const char *str);

		Object * read_numeric_string(char c);
		Object * read_symbol(char c);
		Object * read_quote(void);
		Object * read_list(void);
		Object * read(void);
		void print(Object *obj);
		void win_debug_print(Object *obj);
	};
};

