// <copyright file="Program.h" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

#pragma once
#include "TestBuilder.h"

using namespace Soup::Syntax;
using namespace Soup::Syntax::InnerTree;

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

				// Check that the provided directory exists
				std::filesystem::path directory = args[1];
				if (!std::filesystem::exists(directory))
				{
					throw std::runtime_error("Provided directory does not exist.");
				}

				std::string includeDir = "";
				std::filesystem::path genDir = directory / "gen";
				ProcessDirectory(directory, includeDir, genDir);

				return 0;
			}
			catch (const std::exception& ex)
			{
				std::cout << "ERROR: " << ex.what() << std::endl;
				return -1;
			}
		}

	private:
		static void ProcessDirectory(
			const std::filesystem::path& directory,
			const std::string& includeDir,
			const std::filesystem::path& genDir)
		{
			std::cout << "Directory: " << directory << std::endl;
			for(auto& childItem : std::filesystem::directory_iterator(directory))
			{
				if (childItem.is_directory())
				{
					if (childItem.path() == genDir)
					{
						std::cout << "Skipping output gen folder." << std::endl;
					}
					else
					{
						// Update gen target directory
						auto secondFromLastEntry = --childItem.path().end();
						std::cout << *secondFromLastEntry << std::endl;
						auto childIncludeDir = includeDir + "/" + secondFromLastEntry->string();
						auto childGenDir = genDir / *secondFromLastEntry;
						ProcessDirectory(childItem, childIncludeDir, childGenDir);
					}
				}
				else if (childItem.path().extension() == ".h")
				{
					// Process the C++ file
					ProcessFile(childItem.path(), includeDir, genDir);
				}
			}
		}

		static void ProcessFile(
			const std::filesystem::path& file,
			const std::string& includeDir,
			const std::filesystem::path& genDir)
		{
			try
			{
				std::cout << file << std::endl;
				auto timeStart = std::chrono::high_resolution_clock::now();

				auto sourceFile = std::ifstream(file);
				auto syntaxTree = SyntaxParser::Parse(sourceFile);

				VerifyResult(syntaxTree, file);

				auto timeStop = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(timeStop - timeStart);

				// std::cout << "Done: " << duration.count() << " seconds." << std::endl;

				// Build the collection of test classes
				TestBuilder testBuilder;
				syntaxTree->GetTranslationUnit().Accept(testBuilder);

				// Build up the runner and save it to file
				if (!testBuilder.GetTestClasses().empty())
				{
					auto includeFile = includeDir + "/" + file.filename().string();
					auto targetGenFile = genDir / file.filename().replace_extension(".gen.h"); 
					auto runnerSyntaxTree = BuildTestRunner(testBuilder, includeFile);

					// Write gen file
					std::cout << "GEN: " << targetGenFile << std::endl;
					auto runnerFile = std::ofstream(targetGenFile);
					runnerSyntaxTree->Write(runnerFile);
				}
				else
				{
					std::cout << "No Tests Found." << std::endl;
				}
			}
			catch(const std::exception& e)
			{
				// std::ifstream sourceFile(file);
				// SyntaxParser::PrintAllTokens(sourceFile);
				throw;
			}
		}

		static void VerifyResult(const std::shared_ptr<const SyntaxTree>& syntaxTree, const std::filesystem::path& file)
		{
			// Read the whole file
			std::ifstream sourceFile(file);
			std::string source((std::istreambuf_iterator<char>(sourceFile)),
				std::istreambuf_iterator<char>());

			// Verifiy we can handle this file...
			std::stringstream output;
			syntaxTree->Write(output);
			auto result = output.str();

			if (source != result)
			{
				std::cout << "Actual: " << result << std::endl;
				throw std::runtime_error("Verify output text matches input source.");
				std::cout << "FAILED!" << std::endl;
			}
		}

		static std::shared_ptr<const SyntaxTree> BuildTestRunner(
			TestBuilder& testBuilder,
			const std::string& file)
		{
			// Build up the test runner
			std::vector<std::shared_ptr<const Declaration>> declarations = {};
			for (auto& testClassEntry : testBuilder.GetTestClasses())
			{
				auto& testClass = testClassEntry.second;
				auto testRunnerFunction = BuildTestRunnerFunction(testClass, file);
				declarations.push_back(testRunnerFunction);
			}

			auto translationUnit = SyntaxFactory::CreateTranslationUnit(
				SyntaxFactory::CreateSyntaxList<Declaration>(std::move(declarations)),
				SyntaxFactory::CreateKeywordToken(SyntaxTokenType::EndOfFile));

			return std::make_shared<const SyntaxTree>(
				std::move(translationUnit));
		}

		static std::shared_ptr<const Declaration> BuildTestRunnerFunction(
			const TestClass& testClass,
			const std::string& file)
		{
			// Build up the class type with qualifier
			std::vector<std::shared_ptr<const SyntaxNode>> namespaceIdentifiers = {};
			std::vector<std::shared_ptr<const SyntaxToken>> namespaceSeparators = {};

			for (auto& qualifier : testClass.GetQualifiers())
			{
				namespaceIdentifiers.push_back(
					SyntaxFactory::CreateSimpleIdentifier(
						SyntaxFactory::CreateUniqueToken(SyntaxTokenType::Identifier, qualifier)));
				namespaceSeparators.push_back(
					SyntaxFactory::CreateKeywordToken(SyntaxTokenType::DoubleColon));
			}

			auto testClassType = SyntaxFactory::CreateTypeSpecifierSequence(
				SyntaxFactory::CreateIdentifierType(
					SyntaxFactory::CreateNestedNameSpecifier(
						SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>(
							std::move(namespaceIdentifiers),
							std::move(namespaceSeparators))),
					SyntaxFactory::CreateSimpleIdentifier(
						SyntaxFactory::CreateUniqueToken(
							SyntaxTokenType::Identifier,
							testClass.GetName()))));

			// #include "[TEST_FILE]"
			// auto className = "[CLASS_NAME]"
			// auto testClass = std::make_unique<[CLASS_TYPE]>();
			// TestState state = { 0, 0 };
			auto classNameLiteral = "\"" + testClass.GetName() + "\"";
			std::vector<std::shared_ptr<const Statement>> statements = 
			{
				SyntaxFactory::CreateDeclarationStatement(
					SyntaxFactory::CreateSimpleDeclaration(
						SyntaxFactory::CreateDeclarationSpecifierSequence(
							SyntaxFactory::CreatePrimitiveDataTypeSpecifier(
								PrimitiveDataType::Auto,
								SyntaxFactory::CreateKeywordToken(
									SyntaxTokenType::Auto,
									{
										SyntaxFactory::CreateTrivia("\n"),
										SyntaxFactory::CreateTrivia("	"),
									},
									{}))),
						SyntaxFactory::CreateInitializerDeclaratorList(
							SyntaxFactory::CreateSyntaxSeparatorList<InitializerDeclarator>(
								{
									SyntaxFactory::CreateInitializerDeclarator(
										SyntaxFactory::CreateSimpleIdentifier(
											SyntaxFactory::CreateUniqueToken(
												SyntaxTokenType::Identifier,
												"className",
												{
													SyntaxFactory::CreateTrivia(" "),
												},
												{})),
										SyntaxFactory::CreateValueEqualInitializer(
											SyntaxFactory::CreateKeywordToken(
												SyntaxTokenType::Equal,
												{
													SyntaxFactory::CreateTrivia(" "),
												},
												{}),
											SyntaxFactory::CreateLiteralExpression(
												LiteralType::Integer,
												SyntaxFactory::CreateUniqueToken(
													SyntaxTokenType::StringLiteral,
													classNameLiteral,
													{
														SyntaxFactory::CreateTrivia(" "),
													},
													{})))),
								},
								{})),
						SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Semicolon))),
				SyntaxFactory::CreateDeclarationStatement(
					SyntaxFactory::CreateSimpleDeclaration(
						SyntaxFactory::CreateDeclarationSpecifierSequence(
							SyntaxFactory::CreatePrimitiveDataTypeSpecifier(
								PrimitiveDataType::Auto,
								SyntaxFactory::CreateKeywordToken(
									SyntaxTokenType::Auto,
									{
										SyntaxFactory::CreateTrivia("\n"),
										SyntaxFactory::CreateTrivia("	"),
									},
									{}))),
						SyntaxFactory::CreateInitializerDeclaratorList(
							SyntaxFactory::CreateSyntaxSeparatorList<InitializerDeclarator>(
								{
									SyntaxFactory::CreateInitializerDeclarator(
										SyntaxFactory::CreateSimpleIdentifier(
											SyntaxFactory::CreateUniqueToken(
												SyntaxTokenType::Identifier,
												"testClass",
												{
													SyntaxFactory::CreateTrivia(" "),
												},
												{})),
										SyntaxFactory::CreateValueEqualInitializer(
											SyntaxFactory::CreateKeywordToken(
												SyntaxTokenType::Equal,
												{
													SyntaxFactory::CreateTrivia(" "),
												},
												{}),
											SyntaxFactory::CreateInvocationExpression(
												SyntaxFactory::CreateIdentifierExpression(
													SyntaxFactory::CreateNestedNameSpecifier(
														SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>(
															{
																SyntaxFactory::CreateSimpleIdentifier(
																	SyntaxFactory::CreateUniqueToken(
																		SyntaxTokenType::Identifier,
																		"std",
																		{
																			SyntaxFactory::CreateTrivia(" "),
																		},
																		{})),
															},
															{
																SyntaxFactory::CreateKeywordToken(SyntaxTokenType::DoubleColon),
															})),
													SyntaxFactory::CreateSimpleTemplateIdentifier(
														SyntaxFactory::CreateUniqueToken(SyntaxTokenType::Identifier, "make_shared"),
														SyntaxFactory::CreateKeywordToken(SyntaxTokenType::LessThan),
														SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>(
															{
																testClassType,
															},
															{}),
														SyntaxFactory::CreateKeywordToken(SyntaxTokenType::GreaterThan))),
												SyntaxFactory::CreateKeywordToken(SyntaxTokenType::OpenParenthesis),
												SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>({}, {}),
												SyntaxFactory::CreateKeywordToken(SyntaxTokenType::CloseParenthesis)))),
								},
								{})),
						SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Semicolon))),
				SyntaxFactory::CreateDeclarationStatement(
					SyntaxFactory::CreateSimpleDeclaration(
						SyntaxFactory::CreateDeclarationSpecifierSequence(
							SyntaxFactory::CreateIdentifierType(
								SyntaxFactory::CreateSimpleIdentifier(
									SyntaxFactory::CreateUniqueToken(
										SyntaxTokenType::Identifier,
										"TestState",
										{
											SyntaxFactory::CreateTrivia("\n"),
											SyntaxFactory::CreateTrivia("	"),
										},
										{})))),
						SyntaxFactory::CreateInitializerDeclaratorList(
							SyntaxFactory::CreateSyntaxSeparatorList<InitializerDeclarator>(
								{
									SyntaxFactory::CreateInitializerDeclarator(
										SyntaxFactory::CreateSimpleIdentifier(
											SyntaxFactory::CreateUniqueToken(
												SyntaxTokenType::Identifier,
												"state",
												{
													SyntaxFactory::CreateTrivia(" "),
												},
												{})),
										SyntaxFactory::CreateValueEqualInitializer(
											SyntaxFactory::CreateKeywordToken(
												SyntaxTokenType::Equal,
												{
													SyntaxFactory::CreateTrivia(" "),
												},
												{}),
											SyntaxFactory::CreateInitializerList(
												SyntaxFactory::CreateKeywordToken(
													SyntaxTokenType::OpenBrace,
													{
														SyntaxFactory::CreateTrivia(" "),
													},
													{}),
												SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>(
													{
														SyntaxFactory::CreateLiteralExpression(
															LiteralType::Integer,
															SyntaxFactory::CreateUniqueToken(
																SyntaxTokenType::IntegerLiteral,
																"0",
																{
																	SyntaxFactory::CreateTrivia(" "),
																},
																{})),
														SyntaxFactory::CreateLiteralExpression(
															LiteralType::Integer,
															SyntaxFactory::CreateUniqueToken(
																SyntaxTokenType::IntegerLiteral,
																"0",
																{
																	SyntaxFactory::CreateTrivia(" "),
																},
																{})),
													},
													{
														SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Comma),
													}),
												SyntaxFactory::CreateKeywordToken(
													SyntaxTokenType::CloseBrace,
													{
														SyntaxFactory::CreateTrivia(" "),
													},
													{})))),
								},
								{})),
						SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Semicolon))),
			};

			for (auto& testMethod : testClass.GetTestMethods())
			{
				if (testMethod.IsTheory)
				{
					for (auto& theory : testMethod.Theories)
					{
						// Hack: Create a single argument as a literal from the string
						auto testNameLiteral = "\"" + testMethod.Name + "(" + EscapeString(theory) + ")\"";
						auto parameters = SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>(
							{
								SyntaxFactory::CreateLiteralExpression(
									LiteralType::String,
									SyntaxFactory::CreateUniqueToken(SyntaxTokenType::StringLiteral, theory)),
							}, 
							{});
						auto runTestCall = BuildRunTestCall(
							testMethod.Name,
							std::move(testNameLiteral),
							std::move(parameters));
						statements.push_back(std::move(runTestCall));
					}
				}
				else
				{
					auto testNameLiteral = "\"" + testMethod.Name + "\"";
					auto parameters = SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>({}, {});
					auto runTestCall = BuildRunTestCall(
						testMethod.Name,
						std::move(testNameLiteral),
						std::move(parameters));
					statements.push_back(std::move(runTestCall));
				}
			}

			// Add return "return state;"
			statements.push_back(
				SyntaxFactory::CreateReturnStatement(
					SyntaxFactory::CreateKeywordToken(
						SyntaxTokenType::Return,
						{
							SyntaxFactory::CreateTrivia("\n"),
							SyntaxFactory::CreateTrivia("\n"),
							SyntaxFactory::CreateTrivia("	"),
						},
						{}),
					SyntaxFactory::CreateIdentifierExpression(
						SyntaxFactory::CreateSimpleIdentifier(
							SyntaxFactory::CreateUniqueToken(
								SyntaxTokenType::Identifier,
								"state",
								{
									SyntaxFactory::CreateTrivia(" "),
								},
								{}))),
					SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Semicolon)));

			// #include "[TEST_FILE]"
			// TestState Run[TEST_CLASS]()
			auto testFileInclude = "#include \"" + file + "\"\n";
			auto testClassRunName = "Run" + testClass.GetName();
			auto runnerFunction = SyntaxFactory::CreateFunctionDefinition(
				SyntaxFactory::CreateDeclarationSpecifierSequence(
					SyntaxFactory::CreateIdentifierType(
						SyntaxFactory::CreateSimpleIdentifier(
							SyntaxFactory::CreateUniqueToken(
								SyntaxTokenType::Identifier,
								"TestState",
								{
									SyntaxFactory::CreateTrivia("#pragma once\n"),
									SyntaxFactory::CreateTrivia(testFileInclude),
									SyntaxFactory::CreateTrivia("\n"),
								},
								{})))),
				SyntaxFactory::CreateIdentifierExpression(
					SyntaxFactory::CreateSimpleIdentifier(
						SyntaxFactory::CreateUniqueToken(
							SyntaxTokenType::Identifier,
							testClassRunName,
							{
								SyntaxFactory::CreateTrivia(" "),
							},
							{}))),
				SyntaxFactory::CreateParameterList(
					SyntaxFactory::CreateKeywordToken(SyntaxTokenType::OpenParenthesis),
					SyntaxFactory::CreateSyntaxSeparatorList<Parameter>({}, {}),
					SyntaxFactory::CreateKeywordToken(
						SyntaxTokenType::CloseParenthesis,
						{},
						{
							SyntaxFactory::CreateTrivia(" "),
						})),
				SyntaxFactory::CreateRegularFunctionBody(
					SyntaxFactory::CreateCompoundStatement(
						SyntaxFactory::CreateKeywordToken(
							SyntaxTokenType::OpenBrace,
							{
								SyntaxFactory::CreateTrivia("\n"),
								SyntaxFactory::CreateTrivia(" "),
							},
							{}),
						SyntaxFactory::CreateSyntaxList<Statement>(
							std::move(statements)),
						SyntaxFactory::CreateKeywordToken(
							SyntaxTokenType::CloseBrace,
							{
								SyntaxFactory::CreateTrivia("\n"),
							},
							{}))));

			return runnerFunction;
		}

		static std::shared_ptr<const Statement> BuildRunTestCall(
			const std::string& testName,
			std::string testNameLiteral,
			std::shared_ptr<const SyntaxSeparatorList<SyntaxNode>> parameters)
		{
			// testClass->[TEST_NAME]([PARAMETERS]);
			auto testMemberCall = SyntaxFactory::CreateExpressionStatement(
				SyntaxFactory::CreateInvocationExpression(
					SyntaxFactory::CreateBinaryExpression(
						BinaryOperator::MemberOfPointer,
						SyntaxFactory::CreateIdentifierExpression(
							SyntaxFactory::CreateSimpleIdentifier(
								SyntaxFactory::CreateUniqueToken(
									SyntaxTokenType::Identifier,
									"testClass",
									{
										SyntaxFactory::CreateTrivia(" "),
									},
									{}))),
						SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Arrow),
						SyntaxFactory::CreateIdentifierExpression(
							SyntaxFactory::CreateSimpleIdentifier(
								SyntaxFactory::CreateUniqueToken(SyntaxTokenType::Identifier, testName)))),
					SyntaxFactory::CreateKeywordToken(SyntaxTokenType::OpenParenthesis),
					parameters,
					SyntaxFactory::CreateKeywordToken(SyntaxTokenType::CloseParenthesis)),
				SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Semicolon));

			// state += RunTest(className, "[TEST_NAME_LITERAL]", [&testClass]() { testClass->[TEST_NAME]([PARAMETERS]); });
			auto runTestCall = SyntaxFactory::CreateExpressionStatement(
				SyntaxFactory::CreateBinaryExpression(
					BinaryOperator::AdditionAssignment,
					SyntaxFactory::CreateIdentifierExpression(
						SyntaxFactory::CreateSimpleIdentifier(
							SyntaxFactory::CreateUniqueToken(
								SyntaxTokenType::Identifier,
								"state",
								{
									SyntaxFactory::CreateTrivia("\n"),
									SyntaxFactory::CreateTrivia("	"),
								},
								{}))),
					SyntaxFactory::CreateKeywordToken(
						SyntaxTokenType::PlusEqual,
						{
							SyntaxFactory::CreateTrivia(" "),
						},
						{}),
					SyntaxFactory::CreateInvocationExpression(
						SyntaxFactory::CreateIdentifierExpression(
							SyntaxFactory::CreateNestedNameSpecifier(
								SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>(
									{
										SyntaxFactory::CreateSimpleIdentifier(
											SyntaxFactory::CreateUniqueToken(
												SyntaxTokenType::Identifier,
												"SoupTest",
												{
													SyntaxFactory::CreateTrivia(" "),
												},
												{})),
									},
									{
										SyntaxFactory::CreateKeywordToken(SyntaxTokenType::DoubleColon),
									})),
							SyntaxFactory::CreateSimpleIdentifier(
								SyntaxFactory::CreateUniqueToken(SyntaxTokenType::Identifier, "RunTest"))),
						SyntaxFactory::CreateKeywordToken(SyntaxTokenType::OpenParenthesis),
						SyntaxFactory::CreateSyntaxSeparatorList<SyntaxNode>(
							{
								SyntaxFactory::CreateIdentifierExpression(
									SyntaxFactory::CreateSimpleIdentifier(
										SyntaxFactory::CreateUniqueToken(SyntaxTokenType::Identifier, "className"))),
								SyntaxFactory::CreateIdentifierExpression(
									SyntaxFactory::CreateSimpleIdentifier(
										SyntaxFactory::CreateUniqueToken(
											SyntaxTokenType::Identifier,
											testNameLiteral,
											{
												SyntaxFactory::CreateTrivia(" "),
											},
											{}))),
								SyntaxFactory::CreateLambdaExpression(
									SyntaxFactory::CreateKeywordToken(
										SyntaxTokenType::OpenBracket,
										{
											SyntaxFactory::CreateTrivia(" "),
										},
										{}),
									SyntaxFactory::CreateSyntaxSeparatorList<LambdaCaptureClause>(
										{
											SyntaxFactory::CreateLambdaCaptureClause(
												SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Ampersand),
												SyntaxFactory::CreateUniqueToken(SyntaxTokenType::Identifier, "testClass")),
										},
										{}),
									SyntaxFactory::CreateKeywordToken(SyntaxTokenType::CloseBracket),
									SyntaxFactory::CreateParameterList(
										SyntaxFactory::CreateKeywordToken(SyntaxTokenType::OpenParenthesis),
										SyntaxFactory::CreateSyntaxSeparatorList<Parameter>({}, {}),
										SyntaxFactory::CreateKeywordToken(SyntaxTokenType::CloseParenthesis)),
									SyntaxFactory::CreateCompoundStatement(
										SyntaxFactory::CreateKeywordToken(
											SyntaxTokenType::OpenBrace,
											{
												SyntaxFactory::CreateTrivia(" "),
											},
											{}),
										SyntaxFactory::CreateSyntaxList<Statement>({
											testMemberCall,
										}),
										SyntaxFactory::CreateKeywordToken(
											SyntaxTokenType::CloseBrace,
											{
												SyntaxFactory::CreateTrivia(" "),
											},
											{}))),
							},
							{
								SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Comma),
								SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Comma),
							}),
						SyntaxFactory::CreateKeywordToken(SyntaxTokenType::CloseParenthesis))),
					SyntaxFactory::CreateKeywordToken(SyntaxTokenType::Semicolon));

			return runTestCall;
		}

		static std::string EscapeString(const std::string& value)
		{
			auto result = std::stringstream();
			char previousCharacter = 0;
			for (char character : value)
			{
				// If escape character and it isnt already escaped
				if (character == '\"' && previousCharacter != '\\')
				{
					result << '\\';
				}

				result << character;
				previousCharacter = character;
			}

			return result.str();
		}
	};
}
