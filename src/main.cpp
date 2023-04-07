#include "bot/bot.h"
#include "token/token.h"

int main()
{
	Bot bot(token::token);
	bot.run();
	return 0;
}