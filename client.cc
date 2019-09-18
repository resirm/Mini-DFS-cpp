#include "fileSys.hpp"
#include "commandParser.hpp"

using namespace std;

int main(){
    shared_ptr<CommandParser> cmdParser = CommandParser::getCommandParser();
    cmdParser->work();

    return 0;
}