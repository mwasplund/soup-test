// <copyright file="Program.h" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

#pragma once
#include "TestBuilder.h"

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
                std::cout << "Start Parse: " << file << std::endl;
                auto timeStart = std::chrono::high_resolution_clock::now();

                auto sourceFilePath = std::filesystem::path(file);
                auto syntaxTree = ParseFile(sourceFilePath);

                auto timeStop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(timeStop - timeStart);

                std::cout << "Done: " << duration.count() << " seconds.";
                std::cout << std::endl;

                // Build the collection of test classes
                TestBuilder testBuilder;
                syntaxTree->GetTranslationUnit().Accept(testBuilder);

                // Build up the test runner
                for (auto& testClassEntry : testBuilder.GetTestClasses())
                {
                    auto& testClass = testClassEntry.second;
                    std::cout << "Class: " << testClass.GetName() << std::endl;
                    for (auto& testMethod : testClass.GetFacts())
                    {
                        std::cout << "Method: " << testMethod << std::endl;
                    }
                }

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

            // Verifiy we can handle this file...
            std::stringstream output;
            syntaxTree->Write(output);
            auto result = output.str();

            if(source != result)
            {
                std::cout << "Actual: " << result << std::endl;
                throw std::runtime_error("Verify output text matches input source.");
            }

            return syntaxTree;
        }
    };
}
