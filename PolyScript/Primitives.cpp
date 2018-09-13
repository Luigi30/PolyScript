#include "stdafx.h"
#include "Primitives.h"
#include "Evaluator.h"

namespace PolyScript
{
	namespace Primitives
	{
		// Add a primitive function to the environment.
		void add_primitive(Object *env, const char *name, Primitive *fn) {
			Object *sym = Object::intern(name);
			Object *prim = Object::MakePrimitive(fn);
			Evaluator::add_variable(env, sym, prim);
		}

		// (+ <integer> ...)
		Object *Plus(Object *env, Object *list) {
			
			bool promote_to_float = false;
			double sum = 0;

			for (Object *args = Evaluator::eval_list(env, list); args != Nil; args = args->cdr) {
				
				if (args->car->tag == T_INT)
					sum += args->car->value;

				else if (args->car->tag == T_FLOAT)
				{
					sum += args->car->float_value;
					promote_to_float = true;
				}
					
				else
					error("+ takes only numbers");
			}

			if (promote_to_float)
				return Object::MakeFloat(sum);
			else
				return Object::MakeInt((int)sum);
		}

		// 'expr
		static Object *Quote(Object *env, Object *list) {
			if (Evaluator::list_length(list) != 1)
				error("Malformed quote");
			return list->car;
		}

		// (list expr ...)
		static Object *List(Object *env, Object *list) {
			return Evaluator::eval_list(env, list);
		}

		// (defun <symbol> (<symbol> ...) expr ...)
		static Object *Defun(Object *env, Object *list) {
			return Evaluator::handle_defun(env, list, T_FUNCTION);
		}

		// (lambda (<symbol> ...) expr ...)
		static Object *Lambda(Object *env, Object *list) {
			return Evaluator::handle_function(env, list, T_FUNCTION);
		}

		// (setq <symbol> expr)
		static Object *Setq(Object *env, Object *list) {
			if (Evaluator::list_length(list) != 2 || list->car->tag != T_SYMBOL)
				error("Malformed setq");
			Object *bind = Evaluator::find(env, list->car);
			if (!bind)
				error("Unbound variable %s", list->car->name);
			Object *value = Evaluator::eval(env, list->cdr->car);
			bind->cdr = value;
			return value;
		}

		// (define <symbol> expr)
		static Object *Define(Object *env, Object *list) {
			if (Evaluator::list_length(list) != 2 || list->car->tag != T_SYMBOL)
				error("Malformed setq");
			Object *sym = list->car;
			Object *value = Evaluator::eval(env, list->cdr->car);
			Evaluator::add_variable(env, sym, value);
			return value;
		}

		// (defmacro <symbol> (<symbol> ...) expr ...)
		static Object *Defmacro(Object *env, Object *list) {
			return Evaluator::handle_defun(env, list, T_MACRO);
		}

		// (println expr)
		static Object *Println(Object *env, Object *list) {
			Parser::print(Evaluator::eval(env, list->car));
			printf("\n");
			return Nil;
		}

		static Object *Eq(Object *env, Object *list)
		{
			if (Evaluator::list_length(list) != 2)
				error("Malformed =");

			Object *values = Evaluator::eval_list(env, list);
			
			Object *x = values->car;
			Object *y = values->cdr->car;

			if (x->tag != y->tag)
			{
				error("eq cannot evaluate equality of two different object types");
				return Nil;
			}

			switch (x->tag)
			{
			case T_INT:
			case T_FLOAT:
				return x->value == y->value ? True : Nil;
			default:
				// TODO: other object types
				error("Only numerical equality is currently supported by eq");
				return Nil;
			}
		}

		///////////

		// Add all our primitives to the environment.
		void create_primitives(Object *env)
		{
			add_primitive(env, "plus", Plus);
			add_primitive(env, "quote", Quote);
			add_primitive(env, "list", List);
			add_primitive(env, "defun", Defun);
			add_primitive(env, "lambda", Lambda);
			add_primitive(env, "setq", Setq);
			add_primitive(env, "define", Define);
			add_primitive(env, "defmacro", Defmacro);
			add_primitive(env, "println", Println);
			add_primitive(env, "eq", Eq);
		}
	};
};
