#pragma once

#include <cassert>
#include <cctype>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <cstdarg>

namespace PolyScript
{
	void error(const char *fmt, ...);

	typedef struct Object *Primitive(struct Object *env, struct Object *args);

	extern Object *obarray;

	extern Object *Nil;
	extern Object *Dot;
	extern Object *Cparen;
	extern Object *True;

	extern Object *env;

	void Initialize();
	void EvaluateString(const char *line);

	extern char string_under_evaluation[65536];
	extern int string_pointer;
	extern bool evaluating_a_script;

	typedef enum ObjectTag {
		T_ATOM,
		T_CELL,
		T_PRIMITIVE,
		T_FUNCTION,
		T_MACRO,
		T_ENV,
		T_SPECIAL
	} ObjectTag;

	typedef enum AtomSubtype {
		AT_INT = 1,
		AT_FLOAT,
		AT_SYMBOL,
	} AtomSubtype;

	// Subtypes for TSPECIAL
	typedef enum {
		T_NIL = 1,
		T_DOT,
		T_CPAREN,
		T_TRUE,
	} SpecialSubtype;

	// A Lisp object. Its contents depend on the union type.
	typedef struct Object {
		ObjectTag tag;

		// If an ATOM, it has a subtype
		AtomSubtype atom_subtype;

		// The size of this Object.
		size_t size;

		// The possible values of an Object.
		union
		{
			// T_INT
			int value;

			// T_FLOAT
			double float_value;

			// T_CELL
			struct {
				struct Object *car;
				struct Object *cdr;
			};

			// T_SYMBOL
			char *name;

			// T_PRIMITIVE
			Primitive *fn;

			// T_SPECIAL
			int subtype;

			// T_FUNCTION or T_MACRO
			struct {
				struct Object *params;
				struct Object *body;
				struct Object *env;
			};

			// T_ENV
			struct {
				struct Object *vars;
				struct Object *up;
			};
		};

		bool IsAtomSubtype(AtomSubtype subtype)
		{
			if (tag == T_ATOM && subtype == subtype)
				return true;
			else
				return false;
		}

		// Allocate a new Object.
		static Object *alloc(ObjectTag type, size_t size)
		{
			size += offsetof(Object, value);

			// Allocate an object.
			Object *obj = (Object *)malloc(size);
			obj->tag = type;

			return obj;
		}

		// Constructors.
		static Object *MakeInt(int value)
		{
			Object *r = alloc(T_ATOM, sizeof(int));
			r->atom_subtype = AT_INT;
			r->value = value;
			return r;
		}

		static Object *MakeFloat(double value)
		{
			Object *r = alloc(T_ATOM, sizeof(double));
			r->atom_subtype = AT_FLOAT;
			r->float_value = value;
			return r;
		}

		static Object *MakeSymbol(const char *name) {
			Object *sym = alloc(T_ATOM, strlen(name) + 1);
			sym->atom_subtype = AT_SYMBOL;

			char *dup = _strdup(name);
			sym->name = dup;

			return sym;
		}

		static Object *MakeFunction(ObjectTag type, Object *params, Object *body, Object *env) {
			assert(type == T_FUNCTION || type == T_MACRO);
			Object *r = alloc(type, sizeof(Object *) * 3);
			r->params = params;
			r->body = body;
			r->env = env;
			return r;
		}

		static Object *MakePrimitive(Primitive *fn) {
			Object *r = alloc(T_PRIMITIVE, sizeof(Primitive *));
			r->fn = fn;
			return r;
		}

		static Object *MakeEnv(Object *vars, Object *up) {
			Object *r = alloc(T_ENV, sizeof(Object *) * 2);
			r->vars = vars;
			r->up = up;
			return r;
		}

		static Object *MakeSpecial(SpecialSubtype subtype) {
			Object *r = (Object *)malloc(sizeof(void *) * 2);
			r->tag = T_SPECIAL;
			r->subtype = subtype;
			return r;
		}

		// By convention, this one is just called "cons"
		static Object *cons(Object *car, Object *cdr)
		{
			Object *cell = alloc(T_CELL, sizeof(Object *) * 2);
			cell->car = car;
			cell->cdr = cdr;

			return cell;
		}

		// Returns ((x . y) . a)
		static Object *acons(Object *x, Object *y, Object *a) {
			return cons(cons(x, y), a);
		}

		// May create a new symbol. If there's a symbol with the same name, it will not create a new symbol
		// but return the existing one.
		static Object *intern(const char *name) {
			for (Object *p = obarray; p != Nil; p = p->cdr)
				if (strcmp(name, p->car->name) == 0)
					return p->car;
			Object *sym = Object::MakeSymbol(name);
			obarray = Object::cons(sym, obarray);
			return sym;
		}

	} Object;
};
