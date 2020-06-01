#include <iostream>
#include "lib_playlist.h"

int test1() {
    Player player;

    auto mishmash = player.createPlaylist("mishmash");
    auto armstrong = player.createPlaylist("armstrong");
    auto whatAWonderfulWorld = player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                                                    "I see trees of green, red roses too..."));
    auto helloDolly = player.openFile(File("audio|artist:Louis Armstrong|title:Hello, Dolly!|"
                                           "Hello, Dolly! This is Louis, Dolly"));
    armstrong->add(whatAWonderfulWorld);
    armstrong->add(helloDolly);
    auto direstraits = player.openFile(File("audio|artist:Dire Straits|title:Money for Nothing|"
                                            "Now look at them yo-yo's that's the way you do it..."));
    auto cabaret = player.openFile(File("video|title:Cabaret|year:1972|Qvfcynlvat Pnonerg"));


    mishmash->add(cabaret);
    mishmash->add(armstrong);
    mishmash->add(direstraits, 1);
    mishmash->add(direstraits);

    std::cout << "=== Playing 'mishmash' (default sequence mode)" << std::endl;
    mishmash->play();

    std::cout << "=== Playing 'mishmash' (shuffle mode, seed 0 for std::default_random_engine)" << std::endl;
    mishmash->setMode(createShuffleMode(0));
    mishmash->play();

    std::cout << "=== Playing 'mishmash' (removed cabaret and last direstraits, odd-even mode)" << std::endl;
    mishmash->remove(0);
    mishmash->remove();
    mishmash->setMode(createOddEvenMode());
    mishmash->play();

    std::cout << "=== Playing 'mishmash' (sequence mode, 'armstrong' odd-even mode)" << std::endl;
    armstrong->setMode(createOddEvenMode());
    mishmash->setMode(createSequenceMode());
    mishmash->play();

    try {
        auto unsupported = player.openFile(File("mp3|artist:Unsupported|title:Unsupported|Content"));
    } catch (PlayerException const& e) {
        std::cout << e.what() << std::endl;
    }

    try {
        auto corrupted = player.openFile(File("Corrupt"));
    } catch (PlayerException const& e) {
        std::cout << e.what() << std::endl;
    }

    try {
        auto corrupted = player.openFile(File("audio|artist:Louis Armstrong|title:Hello, Dolly!|%#!@*&"));
    } catch (PlayerException const& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}

int test2() {
	Player player;
	
	auto p1 = player.createPlaylist("ta sama nazwa");
	auto p2 = player.createPlaylist("ta sama nazwa");
	auto audioFile = File("audio|artist:a|title:t|u:|tresc");
	auto audio = player.openFile(audioFile);
	
	p1->add(audio);
	p2->add(audio);
	std::cout << "p1 play" << std::endl;
	p1->play();
	std::cout << "p2 play" << std::endl;
	p2->play();
	p1->add(p2);
	std::cout << "p2 play" << std::endl;
	p2->play();
	std::cout << "p1 play" << std::endl;
	p1->play();
	
	try {
		p2->add(p1);
	} catch (PlayerException const& e) {
		std::cout << e.what() << std::endl;
	}
	
	return 0;
}

int test3() {
	Player player;
	
	PlaylistPtr playlist;
	
	{
		std::string_view buffer_v, buffer2_v;
		
		std::string buffer("audio|artist:a|title:t|content");
		buffer_v = std::string_view(buffer);
		
		auto audio = player.openFile(File(buffer));
		audio->play();
		buffer[13] = 'b';
		audio->play();
		
		std::string buffer2("super nazwa");
		buffer2_v = std::string_view(buffer2);
		
		playlist = player.createPlaylist(buffer2);
		buffer2[0] = 'd';
		playlist->add(audio);
		buffer[13] = 'c';
		playlist->add(audio);
		playlist->play();
		
		std::cout << buffer_v << std::endl;
		std::cout << buffer2_v << std::endl;
	}
	
	playlist->play();
	
	return 0;
}

int main() {
	std::cout << "\nTEST 1\n";
	test1();
	
	std::cout << "\nTEST 2\n";
	test2();
	
	std::cout << "\nTEST 3\n";
	test3();
}
