#include "catch_amalgamated.hpp"
#include "script/msg_macros.h"

static std::string str2escaped(std::string src)
{
    if(src.empty())
        return src;

    static const std::string from = "\n";
    static const std::string to = "\\n";

    size_t start_pos = 0;
    while((start_pos = src.find(from, start_pos)) != std::string::npos)
    {
        src.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }

    return src;
}

void testMessageCondition(const char *line, const std::string &expected, int macro_character, int macro_state)
{
    std::string ret;
    MsgMacroErrors err;
    msgMacroProcess(std::string(line), ret, macro_character, macro_state, &err);

    INFO("Error handling of input:\n------------------------------------\n" << line << "\n------------------------------------\n");
    CAPTURE(str2escaped(line));
    CHECK(err == MSG_MACRO_ERROR_OK);

    INFO("Comparison of:\n-----------Expected:----------------\n" << expected  << "\n-----------Actual:------------------\n" << ret << "\n------------------------------------");
    CAPTURE(str2escaped(ret), str2escaped(expected));
    CHECK(ret == expected);
}

void testMessageValidness(const char *line, int macro_character, int macro_state, MsgMacroErrors expected)
{
    std::string ret;
    MsgMacroErrors err;
    INFO("Error handling of input:\n------------------------------------\n" << line << "\n------------------------------------\n");
    msgMacroProcess(std::string(line), ret, macro_character, macro_state, &err);
    INFO("Expected error code: " << expected << "\nActual error code: " << err);
    CHECK(err == expected);
}


TEST_CASE("[message-box text macros] Ordinary usage")
{
    const char *sample_001 =
        "Hello!\n"
        "#if player(1,2)\n"
        "You are plumber?!\n"
        "#elif player(3)\n"
        "You are princess?!\n"
        "#elif player(4)\n"
        "What the heck yo uare here?! Go clean toilets!\n"
        "#else\n"
        "Welcome to our world, stranger!\n"
        "#endif\n"
        "I have nothing to tell you!";

    SECTION("Sample 001");
    testMessageCondition(sample_001, "Hello!\nYou are plumber?!\nI have nothing to tell you!", 1, -1);
    testMessageCondition(sample_001, "Hello!\nYou are plumber?!\nI have nothing to tell you!", 2, -1);
    testMessageCondition(sample_001, "Hello!\nYou are princess?!\nI have nothing to tell you!", 3, -1);
    testMessageCondition(sample_001, "Hello!\nWhat the heck yo uare here?! Go clean toilets!\nI have nothing to tell you!", 4, -1);
    testMessageCondition(sample_001, "Hello!\nWelcome to our world, stranger!\nI have nothing to tell you!", 5, -1);

    const char *sample_002 =
        "#if player(3)\nХахаха. Ты сюда не вой-\n"
        "дёшь. Эта дверь запечатана магически. Ты должна соб-\n"
        "рать 15 звёзд, чтобы она открылась. Проваливай, малявка!\n"
        "#else\n"
        "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\n"
        "рать 15 звёзд, чтобы она открылась. Проваливай, сопляк!\n"
        "#endif";

    SECTION("Sample 002");
    testMessageCondition(sample_002, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 1, -1);
    testMessageCondition(sample_002, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 2, -1);
    testMessageCondition(sample_002, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должна соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, малявка!", 3, -1);
    testMessageCondition(sample_002, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 4, -1);
    testMessageCondition(sample_002, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 5, -1);
    testMessageCondition(sample_002, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", -1, -1);

    const char *sample_003 =
        "#if player(3)\n"
        "Hey, you! Get off my lawn!\n"
        "I don't care if you're a princess, just, get out!\n"
        "#else\n"
        "You boys better get off my lawn!\n"
        "#endif";
    testMessageCondition(sample_003, "You boys better get off my lawn!", 1, -1);
    testMessageCondition(sample_003, "You boys better get off my lawn!", 2, -1);
    testMessageCondition(sample_003, "Hey, you! Get off my lawn!\nI don't care if you're a princess, just, get out!", 3, -1);
    testMessageCondition(sample_003, "You boys better get off my lawn!", 4, -1);
    testMessageCondition(sample_003, "You boys better get off my lawn!", 5, -1);
    testMessageCondition(sample_003, "You boys better get off my lawn!", -1, -1);


    const char *sample_004 = "#if player(3)\nБудь осторожна,\n#else\nБудь осторожен,\n#endif\nна пути много опасностей!";
    testMessageCondition(sample_004, "Будь осторожен,\nна пути много опасностей!", 1, -1);
    testMessageCondition(sample_004, "Будь осторожен,\nна пути много опасностей!", 2, -1);
    testMessageCondition(sample_004, "Будь осторожна,\nна пути много опасностей!", 3, -1);
    testMessageCondition(sample_004, "Будь осторожен,\nна пути много опасностей!", 4, -1);
    testMessageCondition(sample_004, "Будь осторожен,\nна пути много опасностей!", 5, -1);
    testMessageCondition(sample_004, "Будь осторожен,\nна пути много опасностей!", -1, -1);


    const char *sample_005 = "#if player(3)\n"
        "Oh, Your Highness! It's such an honor to see you, Princess! I have a gift for you, let me open the chest...\n"
        "#elif player(4)\n"
        "Hello, bro! What's up? Take the thing from the chest, you'll need it!\n"
        "#elif player(5)\n"
        "Oh, you that hero from a far kingdom that everyone keep talking about? Here, you can have what's in this chest.\n"
        "#else\n"
        "Aren't you that plumber that everyone keeps talking about? Here, you can have what's in this chest.\n"
        "#endif";
    testMessageCondition(sample_005, "Aren't you that plumber that everyone keeps talking about? Here, you can have what's in this chest.", 1, -1);
    testMessageCondition(sample_005, "Aren't you that plumber that everyone keeps talking about? Here, you can have what's in this chest.", 2, -1);
    testMessageCondition(sample_005, "Oh, Your Highness! It's such an honor to see you, Princess! I have a gift for you, let me open the chest...", 3, -1);
    testMessageCondition(sample_005, "Hello, bro! What's up? Take the thing from the chest, you'll need it!", 4, -1);
    testMessageCondition(sample_005, "Oh, you that hero from a far kingdom that everyone keep talking about? Here, you can have what's in this chest.", 5, -1);
    testMessageCondition(sample_005, "Aren't you that plumber that everyone keeps talking about? Here, you can have what's in this chest.", -1, -1);

    const char *sample_006 = "#if player(3)\n"
        "Did you like my gift?\n"
        "#else\n"
        "I hope that was helpful.\n"
        "#endif";
    testMessageCondition(sample_006, "I hope that was helpful.", 1, -1);
    testMessageCondition(sample_006, "I hope that was helpful.", 2, -1);
    testMessageCondition(sample_006, "Did you like my gift?", 3, -1);
    testMessageCondition(sample_006, "I hope that was helpful.", 4, -1);
    testMessageCondition(sample_006, "I hope that was helpful.", 5, -1);
    testMessageCondition(sample_006, "I hope that was helpful.", -1, -1);

    const char *sample_007 = "#if player(3)\n"
        "О, Ваше Высочество! Для меня такая честь повидать Вас! У меня есть для Вас подарок, позвольте открыть сундук...\n"
        "#elif player(4)\n"
        "Здарова, брательник! Как поживаешь? Возьми кое-что из сундука, это тебе пригодится!\n"
        "#elif player(5)\n"
        "О, ты случайно не тот герой из далёкой страны, про которого все рассказывали? Возьми кое-что из этого сундука.\n"
        "#else\n"
        "Ты случайно не тот санте-\n"
        "хник, о котором все гово-\n"
        "рят? Возьми кое-что из \n"
        "этого сундука.\n"
        "#endif";
    testMessageCondition(sample_007, "Ты случайно не тот санте-\nхник, о котором все гово-\nрят? Возьми кое-что из \nэтого сундука.", 1, -1);
    testMessageCondition(sample_007, "Ты случайно не тот санте-\nхник, о котором все гово-\nрят? Возьми кое-что из \nэтого сундука.", 2, -1);
    testMessageCondition(sample_007, "О, Ваше Высочество! Для меня такая честь повидать Вас! У меня есть для Вас подарок, позвольте открыть сундук...", 3, -1);
    testMessageCondition(sample_007, "Здарова, брательник! Как поживаешь? Возьми кое-что из сундука, это тебе пригодится!", 4, -1);
    testMessageCondition(sample_007, "О, ты случайно не тот герой из далёкой страны, про которого все рассказывали? Возьми кое-что из этого сундука.", 5, -1);
    testMessageCondition(sample_007, "Ты случайно не тот санте-\nхник, о котором все гово-\nрят? Возьми кое-что из \nэтого сундука.", -1, -1);
}

TEST_CASE("[message-box text macros] State function")
{
    const char *sample_001 =
        "#if state(1)\n"
        "Tiny as mouse!\n"
        "#elif state(2)\n"
        "Just a human.\n"
        "#elif state(3)\n"
        "Don't burn me!\n"
        "#elif state(4)\n"
        "So cute raccoon!\n"
        "#elif state(5)\n"
        "Hey, what this statue does here?\n"
        "#elif state(6)\n"
        "Armored warrior!\n"
        "#elif state(7)\n"
        "Freeze me, I want see the future!\n"
        "#else\n"
        "Who are you?!\n"
        "#endif";

    testMessageCondition(sample_001, "Tiny as mouse!", 1, 1);
    testMessageCondition(sample_001, "Just a human.", 1, 2);
    testMessageCondition(sample_001, "Don't burn me!", 1, 3);
    testMessageCondition(sample_001, "So cute raccoon!", 3, 4);
    testMessageCondition(sample_001, "Hey, what this statue does here?", 3, 5);
    testMessageCondition(sample_001, "Armored warrior!", 3, 6);
    testMessageCondition(sample_001, "Freeze me, I want see the future!", 3, 7);
    testMessageCondition(sample_001, "Who are you?!", 3, 8);
    testMessageCondition(sample_001, "Who are you?!", 3, 9);

    const char *sample_002 =
        "You have power of\n"
        "#if_iw state(1)\n"
        "tininess\n"
        "#elif state(2)\n"
        "humanity\n"
        "#elif state(3)\n"
        "fire\n"
        "#elif state(4)\n"
        "tail\n"
        "#elif state(5)\n"
        "fur\n"
        "#elif state(6)\n"
        "armor\n"
        "#elif state(7)\n"
        "ice\n"
        "#else\n"
        "what?\n"
        "#endif_in\n"
        "!";

    testMessageCondition(sample_002, "You have power of tininess!", 1, 1);
    testMessageCondition(sample_002, "You have power of humanity!", 1, 2);
    testMessageCondition(sample_002, "You have power of fire!", 1, 3);
    testMessageCondition(sample_002, "You have power of tail!", 3, 4);
    testMessageCondition(sample_002, "You have power of fur!", 3, 5);
    testMessageCondition(sample_002, "You have power of armor!", 3, 6);
    testMessageCondition(sample_002, "You have power of ice!", 3, 7);
    testMessageCondition(sample_002, "You have power of what?!", 3, 8);
    testMessageCondition(sample_002, "You have power of what?!", 3, 9);
}

TEST_CASE("[message-box text macros] Extra spaces")
{
    const char *sample_001 =
        "Hello!\n"
        "#  if     player(   1 ,      2       )   \n"
        "You are plumber?!\n"
        "# elif             player(  3      )   \n"
        "You are princess?!\n"
        "#          elif player( 4    )  \n"
        "What the heck yo uare here?! Go clean toilets!\n"
        "#    else  \n"
        "Welcome to our world, stranger!\n"
        "#     endif   \n"
        "I have nothing to tell you!";

    testMessageCondition(sample_001, "Hello!\nYou are plumber?!\nI have nothing to tell you!", 1, -1);
    testMessageCondition(sample_001, "Hello!\nYou are plumber?!\nI have nothing to tell you!", 2, -1);
    testMessageCondition(sample_001, "Hello!\nYou are princess?!\nI have nothing to tell you!", 3, -1);
    testMessageCondition(sample_001, "Hello!\nWhat the heck yo uare here?! Go clean toilets!\nI have nothing to tell you!", 4, -1);
    testMessageCondition(sample_001, "Hello!\nWelcome to our world, stranger!\nI have nothing to tell you!", 5, -1);
}

TEST_CASE("[message-box text macros] Non-Line-breaking branches")
{
    const char *sample_001 =
        "Хахаха. Ты сюда не вой-\n"
        "дёшь. Эта дверь запечатана магически. Ты\n"
        "#if_iw player(3)\n"
        "должна\n"
        "#else\n"
        "должен\n"
        "#endif_iw\n"
        "соб-\n"
        "рать 15 звёзд, чтобы она открылась. Проваливай,\n"
        "#if_iw player(3)\n"
        "малявка!\n"
        "#else\n"
        "сопляк!\n"
        "#endif_in";

    testMessageCondition(sample_001, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 1, -1);
    testMessageCondition(sample_001, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 2, -1);
    testMessageCondition(sample_001, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должна соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, малявка!", 3, -1);
    testMessageCondition(sample_001, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 4, -1);
    testMessageCondition(sample_001, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", 5, -1);
    testMessageCondition(sample_001, "Хахаха. Ты сюда не вой-\nдёшь. Эта дверь запечатана магически. Ты должен соб-\nрать 15 звёзд, чтобы она открылась. Проваливай, сопляк!", -1, -1);

    const char *sample_002 =
        "Some phrase with\n"
        "#if_iw player(1)\n"
        "first\n"
        "#else\n"
        "another\n"
        "#endif_iw\n"
        "player.";

    testMessageCondition(sample_002, "Some phrase with first player.", 1, -1);
    testMessageCondition(sample_002, "Some phrase with another player.", 2, -1);
    testMessageCondition(sample_002, "Some phrase with another player.", 3, -1);
    testMessageCondition(sample_002, "Some phrase with another player.", 4, -1);
    testMessageCondition(sample_002, "Some phrase with another player.", 5, -1);
    testMessageCondition(sample_002, "Some phrase with another player.", -1, -1);

    const char *sample_003 =
        "This is mega\n"
        "#if_in player(1)\n"
        "demo\n"
        "#elif player(2)\n"
        "iris\n"
        "#elif player(3)\n"
        "tortoise\n"
        "#elif player(4,5)\n"
        "human\n"
        "#else\n"
        "trash\n"
        "#endif_in\n"
        "!";

    testMessageCondition(sample_003, "This is megademo!", 1, -1);
    testMessageCondition(sample_003, "This is megairis!", 2, -1);
    testMessageCondition(sample_003, "This is megatortoise!", 3, -1);
    testMessageCondition(sample_003, "This is megahuman!", 4, -1);
    testMessageCondition(sample_003, "This is megahuman!", 5, -1);
    testMessageCondition(sample_003, "This is megatrash!", 6, -1);
    testMessageCondition(sample_003, "This is megatrash!", -1, -1);
}

TEST_CASE("[message-box text macros] Error handling")
{
    SECTION("Valid example")
    {
        testMessageValidness("Hello!\n#if player(1)\nYou are plumber?!\n#else\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_OK);
    }

    SECTION("Unknown condition command")
    {
        testMessageValidness("Hello!\n#wtfcond player(1)\nYou are plumber?!\n#else\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_UNKNOWN_CMD);
        testMessageValidness("Hello!\n#if player(1)\nYou are plumber?!\n#what\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_UNKNOWN_CMD);
        testMessageValidness("Hello!\n#if player(1)\nYou are plumber?!\n#else\nWho are you?\n#no you are mess\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_UNKNOWN_CMD);
    }

    SECTION("Unknown function")
    {
        testMessageValidness("Hello!\n#if drillingneighbour(1)\nYou are plumber?!\n#else\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_UNKNOWN_FUNC);
    }


    SECTION("Bad function syntax")
    {
        testMessageValidness("Hello!\n#if player(\nYou are plumber?!\n#elif player(2)\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_SYNTAX);
        testMessageValidness("Hello!\n#if player(1)\nYou are plumber?!\n#elif player)\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_SYNTAX);
        testMessageValidness("Hello!\n#if player)1)\nYou are plumber?!\n#else\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_SYNTAX);
        testMessageValidness("Hello!\n#if state ] 1  \nYou are plumber?!\n#else\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_SYNTAX);
        testMessageValidness("Hello!\n#if player(1 3)\nYou are plumber?!\n#else\nWho are you?\n#no you are mess\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_SYNTAX);
        testMessageValidness("Hello!\n#if player(3 (2 3)\nYou are plumber?!\n#else\nWho are you?\n#no you are mess\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_SYNTAX);
    }

    SECTION("Bad function arguments")
    {
        testMessageValidness("Hello!\n#if player(1x)\nYou are plumber?!\n#else\nWho are you?\n#no you are mess\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_ARGS);
        testMessageValidness("Hello!\n#if player(f, \", 3)\nYou are plumber?!\n#else\nWho are you?\n#no you are mess\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_ARGS);
        testMessageValidness("Hello!\n#if player(fe,(,2, 3)\nYou are plumber?!\n#else\nWho are you?\n#no you are mess\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_ARGS);
        testMessageValidness("Hello!\n#if player(ff (2 3)\nYou are plumber?!\n#else\nWho are you?\n#no you are mess\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_FUNC_ARGS);
    }

    SECTION("Junk at end of line")
    {
        testMessageValidness("Hello!\n#if player(1)мыши\nYou are plumber?!\n#else\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_EXTRA_SYMBOLS_AT_END);
        testMessageValidness("Hello!\n#if player(1)\nYou are plumber?!\n#elif plazer(  3331, 31) , some\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_EXTRA_SYMBOLS_AT_END);
    }

    SECTION("Bad use of else and endif")
    {
        testMessageValidness("Hello!\n#if player\nYou are plumber?!\n#elif player(2)\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_CMD_SYNTAX);
        testMessageValidness("Hello!\n#if player(1)\nYou are plumber?!\n#else player(2)\nWho are you?\n#endif\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_CMD_SYNTAX);
        testMessageValidness("Hello!\n#if player(1)\nYou are plumber?!\n#else\nWho are you?\n#endif player(2)\nI have nothing to tell you!", 1, -1, MSG_MACRO_ERROR_BAD_CMD_SYNTAX);
    }
}
