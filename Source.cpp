#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <iostream>

class TypingGame {
public:
    TypingGame(int limit)
        : score(0), timeLimit(limit), correctWords(0), currentWordIndex(0), timerStarted(false), gameOver(false) {
        words = { "gerry", "kevin", "nibras", "kuntoro", "skill", "focus", "quick", "short",
                  "smart", "train", "words", "clear", "logic", "flash", "check", "sharp",
                  "track", "level", "basic", "value", "write", "limit", "timer", "point" };

        window.create(sf::VideoMode(1000, 600), "Typing Speed Game", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            throw std::runtime_error("Failed to load font file: arial.ttf");
        }

        // Background color setup
        backgroundColor = sf::Color(240, 240, 240);
        wordBackgroundColor = sf::Color(220, 220, 220);

        // Text initialization with improved design
        initializeText(userInputText, font, 35, sf::Color(50, 50, 50), { 500.f, 500.f });
        userInputText.setStyle(sf::Text::Bold);

        initializeText(scoreText, font, 25, sf::Color(70, 70, 70), { 20.f, 20.f });
        initializeText(timerText, font, 25, sf::Color(70, 70, 70), { 20.f, 60.f });
        initializeText(instructionText, font, 20, sf::Color(100, 100, 100), { 500.f, 550.f });
        instructionText.setString("Type the words and press SPACE to submit");
        centerText(instructionText, true);

        initializeText(retryText, font, 30, sf::Color(50, 50, 50), { 500.f, 400.f });
        retryText.setString("Retry");
        centerText(retryText);

        initializeButton(retryButton, { 400.f, 380.f }, { 150.f, 50.f }, "Retry");

        // New reset row button
        initializeResetRowButton();
        generateInitialRows();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();

            if (timerStarted && difftime(time(0), startTime) >= timeLimit) {
                gameOver = true;
                showResults();
            }

            if (!gameOver) {
                renderGame();
            }
        }
    }
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text userInputText, scoreText, timerText, instructionText, retryText;
    sf::RectangleShape retryButton;
    sf::RectangleShape resetRowButton;  
    sf::Text resetRowButtonText;        
    std::vector<std::string> words;
    std::vector<std::vector<sf::Text>> wordRows;

    sf::Color backgroundColor, wordBackgroundColor;

    int score, timeLimit, correctWords, currentWordIndex;
    bool timerStarted, gameOver;
    time_t startTime;
    std::string userInput;

    void initializeText(sf::Text& text, const sf::Font& font, int size, sf::Color color, sf::Vector2f position) {
        text.setFont(font);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(position);
    }

    void initializeButton(sf::RectangleShape& button, sf::Vector2f position, sf::Vector2f size, std::string label) {
        button.setPosition(position);
        button.setSize(size);
        button.setFillColor(sf::Color(200, 200, 200));
        button.setOutlineColor(sf::Color(180, 180, 180));
        button.setOutlineThickness(2.f);

        sf::Text buttonText;
        initializeText(buttonText, font, 24, sf::Color(50, 50, 50), { position.x + size.x / 2, position.y + size.y / 2 });
        buttonText.setString(label);
        centerText(buttonText, true);
        window.draw(buttonText);
    }

    void initializeResetRowButton() {
        // Position the reset row button in a fixed location
        resetRowButton.setPosition({ 920.f, 150.f });
        resetRowButton.setSize({ 70.f, 50.f });
        resetRowButton.setFillColor(sf::Color(200, 200, 200));
        resetRowButton.setOutlineColor(sf::Color(180, 180, 180));
        resetRowButton.setOutlineThickness(2.f);

        // Initialize reset row button text specifically
        initializeText(resetRowButtonText, font, 20, sf::Color(50, 50, 50), { 935.f, 165.f });
        resetRowButtonText.setString("Reset");
    }

    void generateInitialRows() {
        for (int i = 0; i < 3; ++i) {
            generateNewRow();
        }
    }

    void generateNewRow() {
        float startX = 150.f; // Starting position closer to the left edge of the box
        float startY = 185.f + wordRows.size() * 70.f;
        float spacingX = 120.f; // Reduced spacing to fit words in the box

        std::vector<sf::Text> rowWords;
        for (int i = 0; i < 6; ++i) {
            std::string word = words[rand() % words.size()];
            sf::Text wordText;
            initializeText(wordText, font, 30, sf::Color(50, 50, 50), { startX + i * spacingX, startY });
            wordText.setString(word);

            // Check if word goes out of bounds and adjust
            if (wordText.getGlobalBounds().left + wordText.getGlobalBounds().width > 900.f) { // Right edge limit
                wordText.setPosition(900.f - wordText.getGlobalBounds().width, startY);
            }

            rowWords.push_back(wordText);
        }
        wordRows.push_back(rowWords);
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::TextEntered) {
                handleTyping(event.text.unicode);
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    checkWord();
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    // Check if reset row button was clicked
                    if (!gameOver && resetRowButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        resetCurrentRow();
                    }
                    else {
                        handleButtonClick(event.mouseButton.x, event.mouseButton.y);
                    }
                }
            }
        }
    }

    void handleTyping(char inputChar) {
        if (!timerStarted) {
            timerStarted = true;
            startTime = time(0);
        }

        if (inputChar == 8 && !userInput.empty()) {
            userInput.pop_back();
        }
        else if (inputChar >= 32 && inputChar < 128 && inputChar != ' ') {
            userInput += static_cast<char>(inputChar);
        }
    }

    void handleButtonClick(int x, int y) {
        if (gameOver && retryButton.getGlobalBounds().contains(x, y)) {
            resetGame();
        }
    }

    void checkWord() {
        if (userInput == wordRows[0][currentWordIndex].getString()) {
            wordRows[0][currentWordIndex].setFillColor(sf::Color::Green);
            correctWords++;
            score += 10;
            userInput.clear();
            currentWordIndex++;

            if (currentWordIndex >= wordRows[0].size()) {
                shiftRows();
            }
        }
        else {
            wordRows[0][currentWordIndex].setFillColor(sf::Color::Red);
        }
    }

    void resetCurrentRow() {
        // Clear all existing rows
        wordRows.clear();

        // Regenerate all three rows with new words
        for (int i = 0; i < 3; ++i) {
            generateNewRow();
        }

        // Reset current word index
        currentWordIndex = 0;

        // Penalty for resetting the rows
        score = std::max(0, score - 15);  // Increased penalty for resetting all rows
    }

    void shiftRows() {
        currentWordIndex = 0;

        if (wordRows.size() > 1) {
            wordRows[0] = wordRows[1];
            for (auto& word : wordRows[0]) {
                word.move(0.f, -70.f);
            }
        }
        if (wordRows.size() > 2) {
            wordRows[1] = wordRows[2];
            for (auto& word : wordRows[1]) {
                word.move(0.f, -70.f);
            }
        }

        wordRows.pop_back();
        generateNewRow();
    }

    void renderGame() {
        window.clear(backgroundColor);

        sf::RectangleShape wordBox(sf::Vector2f(800.f, 250.f));
        wordBox.setFillColor(wordBackgroundColor);
        wordBox.setPosition(100.f, 150.f);

        // Draw the outline rectangle
        sf::RectangleShape outlineBox(sf::Vector2f(804.f, 254.f));
        outlineBox.setFillColor(sf::Color::Transparent);
        outlineBox.setOutlineColor(sf::Color(180, 180, 180));
        outlineBox.setOutlineThickness(2.f);
        outlineBox.setPosition(98.f, 148.f);
        window.draw(outlineBox);

        window.draw(wordBox);

        int timeRemaining = timerStarted ? timeLimit - static_cast<int>(difftime(time(0), startTime)) : timeLimit;
        scoreText.setString("Score: " + std::to_string(score));
        timerText.setString("Time: " + std::to_string(timeRemaining) + "s");

        for (const auto& row : wordRows) {
            for (const auto& word : row) {
                window.draw(word);
            }
        }

        // Draw reset row button only when not in game over state
        if (!gameOver) {
            window.draw(resetRowButton);
            window.draw(resetRowButtonText);
        }

        userInputText.setString(userInput);
        window.draw(userInputText);
        window.draw(scoreText);
        window.draw(timerText);
        window.draw(instructionText);

        if (gameOver) {
            window.draw(retryText);
            window.draw(retryButton);
        }

        window.display();
    }

    void showResults() {
        window.clear(backgroundColor);

        // Display final results text
        sf::Text resultText;
        initializeText(resultText, font, 35, sf::Color(50, 50, 50), { 500.f, 250.f });
        resultText.setStyle(sf::Text::Bold);
        resultText.setString(
            "Game Over!\n\nFinal Score: " + std::to_string(score) +
            "\nWords Correct: " + std::to_string(correctWords) +
            "\nWPM: " + std::to_string(correctWords * (60 / timeLimit))
        );
        centerText(resultText);
        window.draw(resultText);

        // Set retry button position with padding below the result text
        float padding = 0.f;
        retryButton.setPosition({ 425.f, resultText.getPosition().y + resultText.getLocalBounds().height + padding });

        // Center retry text within the retry button
        retryText.setString("Retry");
        sf::FloatRect textBounds = retryText.getLocalBounds();
        retryText.setOrigin(textBounds.width / 2.f, textBounds.height / 2.f);
        retryText.setPosition(
            retryButton.getPosition().x + retryButton.getSize().x / 2.f,
            retryButton.getPosition().y + retryButton.getSize().y / 2.f
        );

        // Draw the retry button and text
        window.draw(retryButton);
        window.draw(retryText);
        window.display();
    }

    void resetGame() {
        score = 0;
        correctWords = 0;
        currentWordIndex = 0;
        timerStarted = false;
        gameOver = false;
        userInput.clear();
        wordRows.clear();
        generateInitialRows();
    }

    void centerText(sf::Text& text, bool horizontally = false) {
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.width / 2.f, horizontally ? 0 : textRect.height / 2.f);
        text.setPosition(
            horizontally ? window.getSize().x / 2.f : text.getPosition().x,
            horizontally ? text.getPosition().y : window.getSize().y / 2.f
        );
    }
};

int main() {
    try {
        TypingGame game(30);
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}