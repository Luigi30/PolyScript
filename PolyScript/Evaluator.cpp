#include "stdafx.h"
#include "Evaluator.h"

namespace PolyScript
{
	namespace Evaluator
	{
		int list_length(Object *list) {
			int len = 0;
			for (;;) {
				if (list == Nil)
					return len;
				if (list->tag != T_CELL)
					error("length: cannot handle dotted list");
				list = list->cdr;
				len++;
			}
		}

		void add_variable(Object *env, Object *sym, Object *val) {
			env->vars = Object::acons(sym, val, env->vars);
		}

		// Returns a newly created environment frame.
		Object *push_env(Object *env, Object *vars, Object *values) {
			if (list_length(vars) != list_length(values))
				error("Cannot apply function: number of argument does not match");
			Object *map = Nil;
			for (Object *p = vars, *q = values; p != Nil; p = p->cdr, q = q->cdr) {
				Object *sym = p->car;
				Object *val = q->car;
				map = Object::acons(sym, val, map);
			}
			return Object::MakeEnv(map, env);
		}

		// Evaluates the list elements from head and returns the last return value.
		Object *progn(Object *env, Object *list) {
			Object *r = NULL;
			for (Object *lp = list; lp != Nil; lp = lp->cdr)
				r = eval(env, lp->car);
			return r;
		}

		// Evaluates one element of a list and returns it.
		Object * eval_list_element(Object *env, Object *list, int element)
		{
			Object *head = NULL;
			Object *tail = NULL;

			int i = -1;

			for (Object *lp = list; lp != Nil; lp = lp->cdr) {

				i++;

				if (element != i)
				{
					continue;
				}

				Object *tmp = eval(env, lp->car);

				if (error_flag)
					return NULL;

				if (head == NULL) {
					head = tail = Object::cons(tmp, Nil);
				}
				else {
					tail->cdr = Object::cons(tmp, Nil);
					tail = tail->cdr;
				}
			}
			if (head == NULL)
				return Nil;
			return head;
		}

		// Evaluates all the list elements and returns their return values as a new list.
		Object *eval_list(Object *env, Object *list) {
			Object *head = NULL;
			Object *tail = NULL;
			for (Object *lp = list; lp != Nil; lp = lp->cdr) {
				Object *tmp = eval(env, lp->car);

				if (error_flag)
					return NULL;

				if (head == NULL) {
					head = tail = Object::cons(tmp, Nil);
				}
				else {
					tail->cdr = Object::cons(tmp, Nil);
					tail = tail->cdr;
				}
			}
			if (head == NULL)
				return Nil;
			return head;
		}

		bool is_list(Object *obj) {
			return obj == Nil || obj->tag == T_CELL;
		}

		// Apply fn with args.
		Object *apply(Object *env, Object *fn, Object *args) {
			if (!is_list(args))
				error("argument must be a list");
			if (fn->tag == T_PRIMITIVE)
				return fn->fn(env, args);
			if (fn->tag == T_FUNCTION) {
				Object *body = fn->body;
				Object *params = fn->params;
				Object *eargs = eval_list(env, args);
				Object *newenv = push_env(fn->env, params, eargs);
				return progn(newenv, body);
			}
			error("not supported");
			return NULL;
		}

		// Searches for a variable by symbol. Returns null if not found.
		Object *find(Object *env, Object *sym) {
			for (Object *p = env; p; p = p->up) {
				for (Object *cell = p->vars; cell != Nil; cell = cell->cdr) {
					Object *bind = cell->car;
					if (sym == bind->car)
						return bind;
				}
			}
			return NULL;
		}

		// Expands the given macro application form.
		Object *macroexpand(Object *env, Object *obj) {
			if (obj->tag != T_CELL || obj->car->IsAtomSubtype(AT_SYMBOL))
				return obj;
			Object *bind = find(env, obj->car);
			if (!bind || bind->cdr->tag != T_MACRO)
				return obj;
			Object *args = obj->cdr;
			Object *body = bind->cdr->body;
			Object *params = bind->cdr->params;
			Object *newenv = push_env(env, params, args);
			return progn(newenv, body);
		}

		Object *handle_function(Object *env, Object *list, ObjectTag type) {
			if (list->tag != T_CELL || !is_list(list->car) || list->cdr->tag != T_CELL)
				error("Malformed lambda");
			for (Object *p = list->car; p != Nil; p = p->cdr) {
				if (p->car->IsAtomSubtype(AT_SYMBOL))
					error("Parameter must be a symbol");
				if (!is_list(p->cdr))
					error("Parameter list is not a flat list");
			}
			Object *car = list->car;
			Object *cdr = list->cdr;
			return Object::MakeFunction(type, car, cdr, env);
		}

		Object *handle_defun(Object *env, Object *list, ObjectTag type) {
			if (list->car->IsAtomSubtype(AT_SYMBOL) || list->cdr->tag != T_CELL)
				error("Malformed defun");
			Object *sym = list->car;
			Object *rest = list->cdr;
			Object *fn = handle_function(env, rest, type);
			add_variable(env, sym, fn);
			return fn;
		}

		// Evaluates the S expression.
		Object *eval(Object *env, Object *obj) {
			if (error_flag)
				return NULL;

			switch (obj->tag) {

			case T_ATOM:
				switch (obj->atom_subtype)
				{
				case AT_INT:
				case AT_FLOAT:
					return obj; // Self-evaluating
				case AT_SYMBOL:
					// Variable
					Object *bind = find(env, obj);
					if (!bind)
					{
						error("Undefined symbol: %s", obj->name);
						return NULL;
					}
					
					return bind->cdr;
				}

			case T_PRIMITIVE:
			case T_FUNCTION:
			case T_SPECIAL:
				// Self-evaluating objects
				return obj;
			case T_CELL: {
				// Function application form
				Object *expanded = macroexpand(env, obj);
				if (expanded != obj)
					return eval(env, expanded);
				Object *fn = eval(env, obj->car);
				Object *args = obj->cdr;
				if (fn->tag != PolyScript::T_PRIMITIVE && fn->tag != PolyScript::T_FUNCTION)
				{
					error("The head of a list must be a function");
					return NULL;
				}
				return apply(env, fn, args);
			}
			default:
				error("Bug: eval: Unknown tag type: %d", obj->tag);
				return NULL;
			}  
		}
	}
}