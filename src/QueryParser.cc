#include "QueryParser.h"
#include "types.h"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<Token> QueryParser::tokenize(const std::string &query) {
  using std::isspace, std::isalpha;

  std::vector<Token> tokens;

  size_t i = 0;

  auto getWord = [&query, &i]() -> std::string {
    std::string word;
    while (i < query.size()) {
      if (isspace(query[i]) || query[i] == '<' || query[i] == '>' ||
          query[i] == '=' || query[i] == '(' || query[i] == ')' ||
          query[i] == ';') {
        break;
      }

      word += query[i];
      ++i;
    }
    return word;
  };

  auto isNumber = [](const std::string &word) -> char * {
    char *num;
    strtol(word.c_str(), &num, 10);

    return num;
  };

  while (i < query.size()) {
    auto c = static_cast<unsigned char>(query[i]);

    if (isalpha(c) || c == '_') {

      // Word starts with an alphabet or _
      // Likely a SQL keyword | string literal | IDENTIFIER

      std::string word = getWord();

      if (word == "SELECT") {
        tokens.emplace_back("SELECT", TokenType::SELECT);
      } else if (word == "FROM") {
        tokens.emplace_back("FROM", TokenType::FROM);
      } else if (word == "WHERE") {
        tokens.emplace_back("WHERE", TokenType::WHERE);
      } else if (word == "OR") {
        tokens.emplace_back("OR", TokenType::OR);
      } else if (word == "AND") {
        tokens.emplace_back("AND", TokenType::AND);
      } else {
        tokens.emplace_back(word, TokenType::IDENTIFIER);
      }
    } else if (c == '\"') {
      std::string word = getWord();

      if (word.size() == 1 || word.back() != '\"') {
        throw std::runtime_error(
            "[tokenizer] Missing end quotation near position " +
            std::to_string(i));
      }

      std::string literal = word.substr(1, word.size() - 2);
      tokens.emplace_back(literal, TokenType::STRING_LITERAL);
    } else if (std::isdigit(c)) {
      std::string word = getWord();
      tokens.emplace_back(word, TokenType::NUMERIC_LITERAL);
    } else if (isspace(c)) {
      ++i;
    } else {
      if (c == '<') {
        tokens.emplace_back("<", TokenType::LESS_THAN);
        ++i;
      } else if (c == '>') {
        tokens.emplace_back(">", TokenType::GREATER_THAN);
        ++i;
      } else if (c == '=') {
        tokens.emplace_back("=", TokenType::EQUALS);
        ++i;
      } else if (c == '(') {
        tokens.emplace_back("(", TokenType::LPAREN);
        ++i;
      } else if (c == ')') {
        tokens.emplace_back(")", TokenType::RPAREN);
        ++i;
      } else if (c == ';') {
        tokens.emplace_back(";", TokenType::END);
        break;
      } else {
        std::string error = "[tokenizer] Unrecognized symbol (" +
                            std::string(1, c) + ") CODE " +
                            std::to_string((int)c) + " near position " +
                            std::to_string(i);
        throw std::runtime_error(error);
      }
    }
  }

  if (tokens.empty() || tokens.back().type != TokenType::END) {
    throw std::runtime_error("[tokenizer] No semicolon at end of query");
  }

  return tokens;
}
