#pragma once

#include "PolyScript.h"

namespace PolyScript
{
	namespace Evaluator
	{
		Object *eval(Object *env, Object *obj);

		int list_length(Object *list);
		void add_variable(Object *env, Object *sym, Object *val);
		Object *push_env(Object *env, Object *vars, Object *values);
		Object *progn(Object *env, Object *list);
		Object *eval_list(Object *env, Object *list);
		Object *eval_list_element(Object *env, Object *list, int element);
		bool is_list(Object *obj);
		Object *apply(Object *env, Object *fn, Object *args);
		Object *find(Object *env, Object *sym);
		Object *macroexpand(Object *env, Object *obj);
		Object *handle_defun(Object *env, Object *list, ObjectTag type);
		Object *handle_function(Object *env, Object *list, ObjectTag type);
		Object *eval(Object *env, Object *obj);
	}
}

