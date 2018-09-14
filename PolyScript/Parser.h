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
#define STRING_MAX_LEN 2048

		int peek();
		void skip_line();

		Object * read_string();
		Object * read_numeric_string(char c);
		Object * read_symbol(char c);
		Object * read_quote();
		Object * read_list();
		Object * read();
		void print(Object *obj);
		void win_debug_print(Object *obj);
	};
};

