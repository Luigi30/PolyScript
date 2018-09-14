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

PolyScript::Object * PolyScript::env;

bool PolyScript::error_flag;

char PolyScript::string_under_evaluation[65536];
int PolyScript::string_pointer = 0;
bool PolyScript::evaluating_a_script = false;

void PolyScript::error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	//exit(1);

	error_flag = true;
}

void PolyScript::Initialize()
{
	PolyScript::Nil = PolyScript::Object::MakeSpecial(PolyScript::T_NIL);
	PolyScript::Dot = PolyScript::Object::MakeSpecial(PolyScript::T_DOT);
	PolyScript::Cparen = PolyScript::Object::MakeSpecial(PolyScript::T_CPAREN);
	PolyScript::True = PolyScript::Object::MakeSpecial(PolyScript::T_TRUE);

	PolyScript::obarray = PolyScript::Nil;

	env = PolyScript::Object::MakeEnv(PolyScript::Nil, NULL);

	PolyScript::Primitives::create_primitives(env);
}

void PolyScript::EvaluateString(const char *line)
{
	PolyScript::evaluating_a_script = true;
	strcpy(PolyScript::string_under_evaluation, line);

	PolyScript::Object *expr = PolyScript::Parser::read();
	PolyScript::Parser::win_debug_print(PolyScript::Evaluator::eval(PolyScript::env, expr));
	OutputDebugStringW(L"\n");

	PolyScript::evaluating_a_script = false;
}

int main()
{
	PolyScript::Initialize();

	//PolyScript::EvaluateString("(if (eq 4 4) (plus 2 2))");
	//printf("\n");

	while (true)
	{
		PolyScript::error_flag = false;

		printf("> ");
		PolyScript::Object *expr = PolyScript::Parser::read();
		if(!PolyScript::error_flag)
			PolyScript::Parser::print(PolyScript::Evaluator::eval(PolyScript::env, expr));
		printf("\n");

		//PolyScript::Parser::win_debug_print(PolyScript::Evaluator::eval(PolyScript::env, expr));
		//OutputDebugStringW(L"\n");
		
	}

    return 0;
}