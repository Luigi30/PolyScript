#include "stdafx.h"
#include "Primitives.h"
#include "Evaluator.h"

#include <Windows.h>

#define DECLARE_PRIMITIVE_FN(NAME) static Object * NAME (Object *env, Object *list)

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
		DECLARE_PRIMITIVE_FN(Plus) {
			
			bool promote_to_float = false;
			double sum = 0;

			for (Object *args = Evaluator::eval_list(env, list); args != Nil; args = args->cdr) {

				if (error_flag)
					return NULL;
				
				if(args->car->tag != T_ATOM)
					error("+ takes only numbers");

				if (args->car->atom_subtype == AT_INT)
					sum += args->car->int_value;
				else if (args->car->atom_subtype == AT_FLOAT)
				{
					sum += args->car->float_value;
					promote_to_float = true;
				}
			}

			if (promote_to_float)
				return Object::MakeFloat(sum);
			else
				return Object::MakeInt((int)sum);
		}

		// (- <integer> ...)
		DECLARE_PRIMITIVE_FN(Minus) {

			bool first_number = true;
			bool promote_to_float = false;
			double sum = 0;

			for (Object *args = Evaluator::eval_list(env, list); args != Nil; args = args->cdr) {

				if (error_flag)
					return NULL;

				if (args->car->tag != T_ATOM)
					error("- takes only numbers");

				if (first_number)
				{
					if (args->car->atom_subtype == AT_INT)
						sum += args->car->int_value;
					else if (args->car->atom_subtype == AT_FLOAT)
					{
						sum += args->car->float_value;
						promote_to_float = true;
					}

					first_number = false;
				}

				else {
					if (args->car->atom_subtype == AT_INT)
						sum -= args->car->int_value;
					else if (args->car->atom_subtype == AT_FLOAT)
					{
						sum -= args->car->float_value;
						promote_to_float = true;
					}
				}

			}

			if (promote_to_float)
				return Object::MakeFloat(sum);
			else
				return Object::MakeInt((int)sum);
		}

		// (- <integer> ...)
		Object *Multiply(Object *env, Object *list) {

			bool promote_to_float = false;
			double sum = 0;

			for (Object *args = Evaluator::eval_list(env, list); args != Nil; args = args->cdr) {

				if (args->car->tag != T_ATOM)
					error("- takes only numbers");

				else {
					if (args->car->atom_subtype == AT_INT)
						sum *= args->car->int_value;
					else if (args->car->atom_subtype == AT_FLOAT)
					{
						sum *= args->car->float_value;
						promote_to_float = true;
					}
				}

			}

			if (!error_flag)
			{
				if (promote_to_float)
					return Object::MakeFloat(sum);
				else if (!error_flag)
					return Object::MakeInt((int)sum);
			}
			else
				return NULL;
		}

		// 'expr
		DECLARE_PRIMITIVE_FN(Quote) {
			if (Evaluator::list_length(list) != 1)
			{
				error("Malformed quote");
				return NULL;
			}
				
			return list->car;
		}

		// (list expr ...)
		DECLARE_PRIMITIVE_FN(List) {
			return Evaluator::eval_list(env, list);
		}

		// (defun <symbol> (<symbol> ...) expr ...)
		DECLARE_PRIMITIVE_FN(Defun) {
			return Evaluator::handle_defun(env, list, T_FUNCTION);
		}

		// (lambda (<symbol> ...) expr ...)
		DECLARE_PRIMITIVE_FN(Lambda) {
			return Evaluator::handle_function(env, list, T_FUNCTION);
		}

		// (setq <symbol> expr)
		DECLARE_PRIMITIVE_FN(Setq) {
			if (Evaluator::list_length(list) != 2 || (list->car->tag != T_ATOM && list->car->atom_subtype == AT_SYMBOL))
			{
				error("Malformed setq");
				return NULL;
			}
				
			Object *bind = Evaluator::find(env, list->car);
			if (!bind)
			{
				error("Unbound variable %s", list->car->name);
				return NULL;
			}
				
			Object *value = Evaluator::eval(env, list->cdr->car);
			bind->cdr = value;
			return value;
		}

		// (define <symbol> expr)
		DECLARE_PRIMITIVE_FN(Define)
		{
			if (Evaluator::list_length(list) != 2 || (list->car->tag != T_ATOM && list->car->atom_subtype == AT_SYMBOL))
			{
				error("Malformed setq");
				return NULL;
			}
			Object *sym = list->car;
			Object *value = Evaluator::eval(env, list->cdr->car);
			Evaluator::add_variable(env, sym, value);
			return value;
		}

		// (defmacro <symbol> (<symbol> ...) expr ...)
		DECLARE_PRIMITIVE_FN(Defmacro)
		{
			return Evaluator::handle_defun(env, list, T_MACRO);
		}

		// (println expr)
		DECLARE_PRIMITIVE_FN(Println) 
		{
			Parser::print(Evaluator::eval(env, list->car));
			printf("\n");
			return Nil;
		}

		DECLARE_PRIMITIVE_FN(Eq)
		{
			if (Evaluator::list_length(list) != 2)
			{
				error(" Malformed =");
				return NULL;
			}
				

			Object *values = Evaluator::eval_list(env, list);
			
			Object *x = values->car;
			Object *y = values->cdr->car;

			if (x->tag != y->tag)
			{
				error("eq cannot evaluate equality of two different object types");
				return Nil;
			}

			if (x->tag == T_ATOM && y->tag == T_ATOM)
			{
				switch (x->atom_subtype)
				{
				case AT_INT:
				case AT_FLOAT:
					return x->int_value == y->int_value ? True : Nil;
				case AT_SYMBOL:
					return x->name == y->name ? True : Nil;
				default:
					// TODO: other atom types
					error("Only numerical and symbol equality is currently supported by eq");
					return Nil;
				}
			}

			return NULL;
		}

		DECLARE_PRIMITIVE_FN(GreaterThan)
		{
			if (Evaluator::list_length(list) != 2)
			{
				error("Malformed <=");
				return NULL;
			}

			Object *values = Evaluator::eval_list(env, list);

			Object *x = values->car;
			Object *y = values->cdr->car;

			if (x->tag != y->tag)
			{
				error("eq cannot evaluate equality of two different object types");
				return Nil;
			}

			if (x->tag == T_ATOM && y->tag == T_ATOM)
			{
				switch (x->atom_subtype)
				{
				case AT_INT:
				case AT_FLOAT:
					return x->int_value > y->int_value ? True : Nil;
				case AT_SYMBOL:
					return x->name > y->name ? True : Nil;
				}
			}
			else
			{
				error("Only numerical and symbol equality is currently supported by eq");
				return Nil;
			}

			return NULL;
		}

		DECLARE_PRIMITIVE_FN(GreaterThanOrEqual)
		{
			if (Evaluator::list_length(list) != 2)
			{
				error("Malformed <=");
				return NULL;
			}

			Object *values = Evaluator::eval_list(env, list);

			Object *x = values->car;
			Object *y = values->cdr->car;

			if (x->tag != y->tag)
			{
				error("eq cannot evaluate equality of two different object types");
				return Nil;
			}

			if (x->tag == T_ATOM && y->tag == T_ATOM)
			{
				switch (x->atom_subtype)
				{
				case AT_INT:
				case AT_FLOAT:
					return x->int_value >= y->int_value ? True : Nil;
				case AT_SYMBOL:
					return x->name >= y->name ? True : Nil;
				}
			}
			else
			{
				error("Only numerical and symbol equality is currently supported by eq");
				return Nil;
			}

			return NULL;
		}

		DECLARE_PRIMITIVE_FN(LessThan)
		{
			if (Evaluator::list_length(list) != 2)
			{
				error("Malformed <=");
				return NULL;
			}

			Object *values = Evaluator::eval_list(env, list);

			Object *x = values->car;
			Object *y = values->cdr->car;

			if (x->tag != y->tag)
			{
				error("eq cannot evaluate equality of two different object types");
				return Nil;
			}

			if (x->tag == T_ATOM && y->tag == T_ATOM)
			{
				switch (x->atom_subtype)
				{
				case AT_INT:
				case AT_FLOAT:
					return x->int_value < y->int_value ? True : Nil;
				case AT_SYMBOL:
					return x->name < y->name ? True : Nil;
				}
			}
			else
			{
				error("Only numerical and symbol equality is currently supported by eq");
				return Nil;
			}

			return NULL;
		}

		DECLARE_PRIMITIVE_FN(LessThanOrEqual)
		{
			if (Evaluator::list_length(list) != 2)
			{
				error("Malformed <=");
				return NULL;
			}

			Object *values = Evaluator::eval_list(env, list);

			Object *x = values->car;
			Object *y = values->cdr->car;

			if (x->tag != y->tag)
			{
				error("eq cannot evaluate equality of two different object types");
				return Nil;
			}

			if (x->tag == T_ATOM && y->tag == T_ATOM)
			{
				switch (x->atom_subtype)
				{
				case AT_INT:
				case AT_FLOAT:
					return x->int_value <= y->int_value ? True : Nil;
				case AT_SYMBOL:
					return x->name <= y->name ? True : Nil;
				}
			}
			else
			{
				error("Only numerical and symbol equality is currently supported by eq");
				return Nil;
			}

			return NULL;
		}

		// Predicates
		DECLARE_PRIMITIVE_FN(SymbolP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (val->IsAtomSubtype(AT_SYMBOL))
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(AtomP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (val->tag == T_ATOM)
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(ConsP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (val->tag == T_CELL)
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(NumberP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (val->IsAtomSubtype(AT_FLOAT) || val->IsAtomSubtype(AT_INT))
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(FloatP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (val->IsAtomSubtype(AT_FLOAT))
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(IntegerP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (val->IsAtomSubtype(AT_INT))
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(ZeroP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (!val->IsNumber())
			{
				error("Argument is not a number");
				return NULL;
			}
			else if (val->IsAtomSubtype(AT_INT) && val->int_value == 0)
				return True;
			else if (val->IsAtomSubtype(AT_FLOAT) && val->float_value == 0)
				return True;
			else 
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(PlusP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (!val->IsNumber())
			{
				error("Argument is not a number");
				return NULL;
			}
			else if (val->IsAtomSubtype(AT_INT) && val->int_value > 0)
				return True;
			else if (val->IsAtomSubtype(AT_FLOAT) && val->float_value > 0)
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(MinusP)
		{
			Object *values = Evaluator::eval_list(env, list);

			if (Evaluator::list_length(list) != 1)
			{
				error("Too many arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			Object *val = values->car;
			if (!val->IsNumber())
			{
				error("Argument is not a number");
				return NULL;
			}
			else if (val->IsAtomSubtype(AT_INT) && val->int_value < 0)
				return True;
			else if (val->IsAtomSubtype(AT_FLOAT) && val->float_value < 0)
				return True;
			else
				return Nil;
		}

		DECLARE_PRIMITIVE_FN(If)
		{
			// Evaluate the first form.
			// If the first form is not nil, evaluate the second form.
			// If the first form is nil, evaluate the third form if one exists.

			int elements = Evaluator::list_length(list);

			if (elements != 2 && elements != 3)
			{
				error("Incorrect number of arguments: found %d instead of 1", Evaluator::list_length(list));
				return NULL;
			}

			// Evaluate the first form.
			Object *first_form = Evaluator::eval_list_element(env, list, 0);

			if (first_form->car->subtype == T_TRUE)
			{
				return Evaluator::eval_list_element(env, list, 1);
			}
			else if (first_form->car->subtype == T_NIL)
			{
				if (elements == 3)
					return Evaluator::eval_list_element(env, list, 2);
				else
					return Nil;
			}
		}

		///////////

		// Add all our primitives to the environment.
		void create_primitives(Object *env)
		{
			// Mathematical primitives
			add_primitive(env, "plus", Plus);
			add_primitive(env, "minus", Minus);
			add_primitive(env, "multiply", Multiply);

			// Lisp primitives
			add_primitive(env, "quote", Quote);
			add_primitive(env, "list", List);
			add_primitive(env, "defun", Defun);
			add_primitive(env, "lambda", Lambda);
			add_primitive(env, "setq", Setq);
			add_primitive(env, "define", Define);
			add_primitive(env, "defmacro", Defmacro);
			add_primitive(env, "println", Println);

			// Equality primitives
			add_primitive(env, "eq", Eq);
			add_primitive(env, ">", GreaterThan);
			add_primitive(env, "<", LessThan);
			add_primitive(env, ">=", GreaterThanOrEqual);
			add_primitive(env, "<=", LessThanOrEqual);

			// Type primitives
			add_primitive(env, "numberp", NumberP);
			add_primitive(env, "floatp", FloatP);
			add_primitive(env, "integerp", IntegerP);
			add_primitive(env, "zerop", ZeroP);
			add_primitive(env, "plusp", PlusP);
			add_primitive(env, "minusp", MinusP);

			add_primitive(env, "if", If);
			
		}
	};
};
