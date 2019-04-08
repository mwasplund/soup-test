// <copyright file="Program.h" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

#pragma once

namespace Soup::Test
{
    /// <summary>
    /// The root of all evil
    /// </summary>
    class Program
    {
    public:
        /// <summary>
        /// The main entry point of the program
        /// </summary>
        static int Main(std::vector<std::string> args)
        {
            try
            {
                if (args.size() != 2)
                {
                    throw std::runtime_error("Expected exactly one argument.");
                }

                auto& file = args[1];
                std::cout << "Start: " << file << std::endl;

                auto sourceFilePath = std::filesystem::path(file);
                auto syntaxTree = ParseFile(sourceFilePath);

                return 0;
            }
            catch (const std::exception& ex)
            {
                std::cout << "ERROR: " << ex.what() << std::endl;
                return -1;
            }
        }

    private:
        static std::shared_ptr<const Soup::Syntax::SyntaxTree> ParseFile(std::filesystem::path& file)
        {
            std::ifstream sourceFile(file);

            // Read the whole file
            sourceFile.seekg(0, std::ios::end);
            size_t size = sourceFile.tellg();
            std::string source(size, ' ');
            sourceFile.seekg(0);
            sourceFile.read(&source[0], size); 

            // Soup::Syntax::SyntaxParser::PrintAllTokens(source);
            auto syntaxTree = Soup::Syntax::SyntaxParser::Parse(source);

            return syntaxTree;
        }
    };
}
