#include "utf8.h"

#include "fmt/core.h"
#include "fmt/color.h"

#include "replxx.hxx"

#include "ilang/Parser.hpp"
#include "ilang/Eval.hpp"

using namespace ilang;

void coloredPrint(const TypeData &types, const std::string &str){
	auto defaultColor = fmt::color::white;
	
	std::vector<Token> toks;
	try{
		toks = lexAll(str);
	}
	catch(...){
		fmt::print("{}\n", str);
		return;
	}
	
	for(auto &&tok : toks){
		switch(tok.type){
			case TokenType::COUNT:
			case TokenType::eof:
			case TokenType::empty:
				break;
			
			case TokenType::newLine: fmt::print("\n"); break;
				
			case TokenType::listL: fmt::print(fmt::fg(defaultColor), "["); break;
			case TokenType::listR: fmt::print(fmt::fg(defaultColor), "]"); break;
			case TokenType::groupL: fmt::print(fmt::fg(defaultColor), "("); break;
			case TokenType::groupR: fmt::print(fmt::fg(defaultColor), ")"); break;
			
			case TokenType::space:{
				fmt::print(fmt::fg(defaultColor), "{}", tok.value);
				break;
			};
			
			case TokenType::id:{
				fmt::color idColor = defaultColor;
				auto type = findTypeByString(types, tok.value);
				if(type) idColor = fmt::color::dark_orange;
				
				fmt::print(fmt::fg(idColor), "{}", tok.value);
				
				break;
			}
			
			case TokenType::real:
			case TokenType::int_:{
				fmt::print(fmt::fg(fmt::color::light_green), "{}", tok.value);
				break;
			}
			
			case TokenType::str:{
				fmt::print(fmt::fg(fmt::color::magenta), "\"{}\"", tok.value);
				break;
			}
			
			case TokenType::op:{
				fmt::print(fmt::fg(defaultColor), "{}", tok.value);
				break;
			}
		}
	}
	
	fmt::print("\n");
}

int main(int arg, char *argv[]){
	EvalResult evalResult;
	EvalData evalData;
	Ast ast;
	
	replxx::Replxx repl;
	
	repl.set_completion_callback([](const std::string &input, int &contextLen){
		replxx::Replxx::completions_t completions;
		return completions;
	});
	
	repl.set_highlighter_callback([&evalData](const std::string &input, replxx::Replxx::colors_t &colors){
		int colorIdx = 0;
		
		auto colorTok = [&colors, &colorIdx](std::string_view val, replxx::Replxx::Color color){
			auto it = begin(val);
			auto end = std::end(val);
			while(it != end){
				utf8::next(it, end);
				colors[colorIdx++] = color;
			}
		};
		
		std::vector<Token> toks;
		try{
			toks = lexAll(input);
		}
		catch(...){
			colorTok(input, replxx::Replxx::Color::DEFAULT);
			return;
		}
		
		for(auto &&tok : toks){
			switch(tok.type){
				case TokenType::COUNT:
				case TokenType::eof:
				case TokenType::empty:
				case TokenType::newLine:
					break;
				
				case TokenType::listL:
				case TokenType::listR:
				case TokenType::groupL:
				case TokenType::groupR:{
					colors[colorIdx++] = replxx::Replxx::Color::WHITE;
					break;
				}
				
				case TokenType::space:{
					colorTok(tok.value, replxx::Replxx::Color::DEFAULT);
					break;
				};
				
				case TokenType::id:{
					replxx::Replxx::Color tokColor = replxx::Replxx::Color::CYAN;
					
					auto type = findTypeByString(evalData.typeData, tok.value);
					if(type)
						tokColor = replxx::Replxx::Color::RED;
					
					colorTok(tok.value, tokColor);
					break;
				}
				
				case TokenType::real:
				case TokenType::int_:{
					colorTok(tok.value, replxx::Replxx::Color::GREEN);
					break;
				}
				
				case TokenType::str:{
					colors[colorIdx++] = replxx::Replxx::Color::MAGENTA; // opening quote
					colorTok(tok.value, replxx::Replxx::Color::MAGENTA);
					colors[colorIdx++] = replxx::Replxx::Color::MAGENTA; // closing quote
					break;
				}
				
				case TokenType::op:{
					colorTok(tok.value, replxx::Replxx::Color::WHITE);
					break;
				}
			}
		}
	});

	std::vector<ResultPtr> prevResults;
	
	bool running = true;
	
	std::function<void()> exitFn = [&running](){
		running = false;
	};
	
	ResultPtr callableExit;
	ResultHandle versionHandle;
	std::tie(callableExit, evalData) = registerEvalFn("exit", std::move(exitFn), std::move(evalData));
	std::tie(versionHandle, evalData) = bindEvalName("version", std::string("0.0.1"), std::move(evalData));

	while(running){
		std::string input = repl.input("> ");
		repl.history_add(input);
		
		if(input == "exit" || input == "quit")
			break;

		//try{
			ast = parseAll(input, evalData.typeData, std::move(ast));

			for(auto expr : ast.root){
				if(!expr) continue;
				std::tie(evalResult, evalData) = eval({expr}, std::move(evalData));
				prevResults.emplace_back(std::move(evalResult.result));
			}

			auto res = prevResults.back().get();
			auto type = res->resolveType(evalData.typeData);
					
			coloredPrint(evalData.typeData, fmt::format("{}", type->str));
			coloredPrint(evalData.typeData, fmt::format("  => {}\n", res->toString()));
		/*}
		catch(const LexError &lexError){
			for(std::size_t i = 0; i < lexError.location().col; i++){
				std::fputc(' ', stderr);
			}
			
			fmt::print(stderr, "^^^\n");
			fmt::print(stderr, fmt::fg(fmt::color::red), "Lexer Error: {}\n", lexError.what());
		}
		catch(const ParseError &parseError){
			fmt::print(stderr, fmt::fg(fmt::color::red), "Parser Error: {}\n", parseError.what());
		}
		catch(const EvalError &evalError){
			fmt::print(stderr, fmt::fg(fmt::color::red), "Evaluation Error: {}\n", evalError.what());
		}
		catch(...){
			throw;
		}*/
		
		/*
		fmt::print(fmt::fg(fmt::color::yellow), "{}\n", type->str);
		fmt::print(fmt::fg(fmt::color::cyan), "=> {}", res->toString());
		fmt::print(fmt::fg(fmt::color::white), "\n");
		*/
	}

	return 0;
}
