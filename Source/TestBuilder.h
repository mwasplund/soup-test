// <copyright file="TestBuilder.h" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

#pragma once

using namespace Soup::Syntax;
using namespace Soup::Syntax::OuterTree;

namespace Soup::Test
{
    /// <summary>
    /// A test class container
    /// </summary>
    class TestClass
    {
    public:
        TestClass(std::string name) :
            m_name(std::move(name)),
            m_facts()
        {
        }

        const std::string& GetName() const
        {
            return m_name;
        }

        const std::vector<std::string>& GetFacts() const
        {
            return m_facts;
        }

        std::vector<std::string>& GetFacts()
        {
            return m_facts;
        }

    private:
        std::string m_name;
        std::vector<std::string> m_facts;
    };

    /// <summary>
    /// Syntax Visitor used to find all test methods
    /// </summary>
    class TestBuilder : public SyntaxWalker
    {
    public:
        TestBuilder()
        {
        }

        const std::map<std::string, TestClass>& GetTestClasses()
        {
            return m_testClasses;
        }

    protected:
        virtual void Visit(const FunctionDefinition& node) override final
        {
            // Check if the function has a parent class
            if (IsFact(node))
            {
                AddTestMethod(node);
            }

            // Call base implementation
            SyntaxWalker::Visit(node);
        }

    private:
        // Check if the privided function has a fact attribute
        void AddTestMethod(const FunctionDefinition& function)
        {
            // Get the parent class name
            auto& parentClass = dynamic_cast<const ClassDeclaration&>(function.GetParent());
            if (!parentClass.HasIdentifierToken())
                throw std::runtime_error("A test class must have a name.");
            auto& parentClassName = parentClass.GetIdentifierToken().GetValue();

            // Ensure that the class is registered
            auto classEntry = m_testClasses.find(parentClassName);
            if (classEntry == m_testClasses.end())
            {
                // Add the new class
                auto insertResult = m_testClasses.emplace(parentClassName, TestClass(parentClassName));
            }

            auto& testClass = m_testClasses.at(parentClassName);

            // Register the method name
            auto& methodName = dynamic_cast<const SimpleIdentifierExpression&>(function.GetIdentifier())
                .GetIdentifierToken().GetValue();
            testClass.GetFacts().push_back(methodName);
        }

        // Check if the privided function has a fact attribute
        bool IsFact(const FunctionDefinition& function)
        {
            auto& attributeSpecifiers = function.GetAttributeSpecifierSequence().GetItems();
            for (auto& specifier : attributeSpecifiers)
            {
                auto attributes = specifier->GetAttributes().GetItems();
                if (attributes.size() == 1)
                {
                    auto& attribute = attributes.at(0);
                    auto& value = attribute->GetIdentifierToken().GetValue();
                    if (value == "Fact")
                    {
                        return true;
                    }
                }
            }

            return false;
        }

    private:
        std::map<std::string, TestClass> m_testClasses;
    };
}
