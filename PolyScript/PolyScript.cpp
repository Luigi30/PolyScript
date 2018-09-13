// PolyScript.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PolyScript.h"
#include "Primitives.h"
#include "Parser.h"
#include "Evaluator.h"

PolyScript::Object * PolyScript::obarray;

PolyScript::Object * PolyScript::Nil;
PolyScript::Object * PolyScript::Dot;
PolyScript::Object * PolyScript::Cparen;
PolyScript::Object * PolyScript::True;

void PolyScript::error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

int main()
{
	PolyScript::Nil = PolyScript::Object::MakeSpecial(PolyScript::T_NIL);
	PolyScript::Dot = PolyScript::Object::MakeSpecial(PolyScript::T_DOT);
	PolyScript::Cparen = PolyScript::Object::MakeSpecial(PolyScript::T_CPAREN);
	PolyScript::True = PolyScript::Object::MakeSpecial(PolyScript::T_TRUE);
	
	PolyScript::obarray = PolyScript::Nil;

	PolyScript::Object *env = PolyScript::Object::MakeEnv(PolyScript::Nil, NULL);

	PolyScript::Primitives::create_primitives(env);

	while (true)
	{
		printf("> ");
		PolyScript::Object *expr = PolyScript::Parser::read();
		PolyScript::Parser::print(PolyScript::Evaluator::eval(env, expr));

		printf("\n");
	}

    return 0;
}