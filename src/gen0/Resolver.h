#pragma once

#include <string>
#include <memory>
#include <sockpp/socket.h>


void Resolver(std::string name, std::shared_ptr<sockpp::socket> answerSocket);