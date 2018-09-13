#pragma once

#include "PolyScript.h"
#include "Evaluator.h"
#include "Parser.h"

namespace PolyScript
{
	namespace Primitives
	{
		void create_primitives(Object *env);

		// Add a primitive function to the environment.
		void add_primitive(Object *env, const char *name, Primitive *fn);
	};
};