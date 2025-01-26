﻿// <copyright file="Main.cpp" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

import SoupSyntaxParser;

#include "Program.h"

int main(int argc, char** argv)
{
	std::vector<std::string> args;
	for (int i = 0; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	return Soup::Test::Program::Main(std::move(args));
}
