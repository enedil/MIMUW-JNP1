#include "lib_playlist.h"
#include <iostream>

void example_test() {
    Player player;

    auto mishmash = player.createPlaylist("mishmash");
    auto armstrong = player.createPlaylist("armstrong");
    auto whatAWonderfulWorld =
        player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                             "I see trees of green, red roses too..."));
    auto helloDolly =
        player.openFile(File("audio|artist:Louis Armstrong|title:Hello, Dolly!|"
                             "Hello, Dolly! This is Louis, Dolly"));
    armstrong->add(whatAWonderfulWorld);
    armstrong->add(helloDolly);
    auto direstraits =
        player.openFile(File("audio|artist:Dire Straits|title:Money for Nothing|"
                             "Now look at them yo-yo's that's the way you do it..."));
    auto cabaret =
        player.openFile(File("video|title:Cabaret|year:1972|Qvfcynlvat Pnonerg"));

    mishmash->add(cabaret);
    mishmash->add(armstrong);
    mishmash->add(direstraits, 1);
    mishmash->add(direstraits);

    std::cout << "=== Playing 'mishmash' (default sequence mode)" << std::endl;
    mishmash->play();

    std::cout
        << "=== Playing 'mishmash' (shuffle mode, seed 0 for std::default_random_engine)"
        << std::endl;
    mishmash->setMode(createShuffleMode(0));
    mishmash->play();

    std::cout
        << "=== Playing 'mishmash' (removed cabaret and last direstraits, odd-even mode)"
        << std::endl;
    mishmash->remove(0);
    mishmash->remove();
    mishmash->setMode(createOddEvenMode());
    mishmash->play();

    std::cout << "=== Playing 'mishmash' (sequence mode, 'armstrong' odd-even mode)"
              << std::endl;
    armstrong->setMode(createOddEvenMode());
    mishmash->setMode(createSequenceMode());
    mishmash->play();

    try {
        auto unsupported =
            player.openFile(File("mp3|artist:Unsupported|title:Unsupported|Content"));
	printf("Error in example_test.");
	exit(-1);
    } catch (PlayerException const &e) {
        std::cout << e.what() << std::endl;
    }

    try {
        auto corrupted = player.openFile(File("Corrupt"));
	printf("Error in example_test.");
	exit(-1);
    } catch (PlayerException const &e) {
        std::cout << e.what() << std::endl;
    }

    try {
        auto corrupted = player.openFile(
            File("audio|artist:Louis Armstrong|title:Hello, Dolly!|%#!@*&"));
	printf("Error in example_test.");
	exit(-1);
    } catch (PlayerException const &e) {
        std::cout << e.what() << std::endl;
    }
}

void self_loop_test() {
	Player player;
	auto p = player.createPlaylist("p");
	try {
		p -> add(p);
		printf("Error in self_loop_test.");
		exit(-1);
	}
	catch (PlayerException const &e) {
		std::cout << e.what() << std::endl;
	}
}

void loop_test() {
	Player player;
	auto p = player.createPlaylist("p");
	auto p2 = player.createPlaylist("p2");
	p -> add(p2);
	try {
		p2 -> add(p);
		printf("Error in loop_test.");
		exit(-1);
	}
	catch (PlayerException const &e) {
		std::cout << e.what() << std::endl;
	}
	p -> add(p2);
}

void odd_even_basic() {
	Player player;
	auto p = player.createPlaylist("p");
	auto whatAWonderfulWorld =
        	player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                             "I see trees of green, red roses too..."));
	p -> add(whatAWonderfulWorld);
	p -> add(whatAWonderfulWorld);
	p -> add(whatAWonderfulWorld);
	p -> setMode(createOddEvenMode());
	p -> play();
}

void modes_changing() {
	Player player;
	auto p = player.createPlaylist("p");
	auto whatAWonderfulWorld =
        	player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                             "I see trees of green, red roses too..."));
	p -> add(whatAWonderfulWorld);
	p -> add(whatAWonderfulWorld);
	p -> add(whatAWonderfulWorld);

	for(int i=0;i<10;i++) {
		p -> setMode(createOddEvenMode());
		p -> play();
		p -> setMode(createSequenceMode());
		p -> play();
		p -> setMode(createShuffleMode(i));
		p -> play();
	}
}

void positions_test() {
	Player player;
	auto p = player.createPlaylist("p");
	auto whatAWonderfulWorld =
        	player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                             "I see trees of green, red roses too..."));
	try {
		p -> remove();
		printf("Error in positions_test");
		exit(-1);
	}
	catch (PlayerException const &e) {
		std::cout << e.what() << std::endl;
	}

	p -> add(whatAWonderfulWorld);
	p -> remove();

	try {
		p -> remove();
		printf("Error in positions_test");
		exit(-1);
	}
	catch (PlayerException const &e) {
		std::cout << e.what() << std::endl;
	}

	for(int i=0;i<10;i++) {
		try {
			p -> remove(i);
			printf("Error in positions_test");
			exit(-1);
		}
		catch (PlayerException const &e) {
			std::cout << e.what() << std::endl;
		}
		p -> add(whatAWonderfulWorld);
	}

	p -> add(whatAWonderfulWorld, 3);
	p -> add(whatAWonderfulWorld, 7);
	p -> add(whatAWonderfulWorld, 11);

	try {
		p -> add(whatAWonderfulWorld, 20);
		printf("Error in positions_test");
		exit(-1);
	}
	catch (PlayerException const &e) {
		std::cout << e.what() << std::endl;
	}

	for(int i=0;i<13;i++) p -> remove();

	try {
		p -> remove();
		printf("Error in positions_test");
		exit(-1);
	}
	catch (PlayerException const &e) {
		std::cout << e.what() << std::endl;
	}

	p -> add(whatAWonderfulWorld, 0);
	p -> remove();
}

void deep_loop() {
	Player player;
	auto p = player.createPlaylist("p");
	auto p2 = p;
	for(int i=0;i<1000;i++) {
		auto p3 = player.createPlaylist("x");
		p2 -> add(p3);
		p2 = p3;
	}
	try {
		p2 -> add(p);
		printf("Error in deep_loop");
		exit(-1);
	}
	catch (PlayerException const &e) {
		std::cout << e.what() << std::endl;
	}
}

void often_changing() {
	Player player;
	auto p = player.createPlaylist("p");
	auto whatAWonderfulWorld =
        player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                             "I see trees of green, red roses too..."));
	p -> add(whatAWonderfulWorld);
	for(int i=0;i<100;i++) {
		p -> setMode(createShuffleMode(i));
		p -> add(whatAWonderfulWorld);
		p -> play();
	}
}

int main() {
	example_test();
	self_loop_test();
	loop_test();
	odd_even_basic();
	modes_changing();
	positions_test();
	deep_loop();
	often_changing();
    return 0;
}
