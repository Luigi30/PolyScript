#pragma once

#include "PolyScript.h"

#include <string>

namespace PolyScript
{
	namespace Parser
	{
#define SYMBOL_MAX_LEN 200
#define NUMBER_MAX_LEN 100

		int peek();
		void skip_line(void);

		Object * read_numeric_string(char c);
		Object * read_symbol(char c);
		Object * read_quote(void);
		Object * read_list(void);
		Object * read(void);
		void print(Object *obj);
	};
};

