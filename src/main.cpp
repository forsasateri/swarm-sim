#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <optional>
#include <memory>

#include "walker/randomWalker.hpp"
#include "world/world.hpp"
#include "util/logger.hpp"
#include "world/coordinateSystem.hpp"

// TODO: Fixed TICK rate
// Second info screen for selected unit


void drawWalkers(sf::RenderWindow& window, std::vector<RandomWalker>& walkers) {
	sf::VertexArray vertexArray(sf::PrimitiveType::Triangles);
	
	for ( auto& w : walkers ) {
		// w.display( window );
		w.addVertexesToArray( vertexArray );
	}

	window.draw( vertexArray );
}

const uint32_t FRAME_LIMIT = 120;
const int TICK_RATE = 20;
const float TICK_DURATION = 1.f / TICK_RATE; // Duration of each tick
// TODO: Logic tick speed
// Provide split between simple movement and complex logic


int main()
{

	sf::Vector2i windowSize = { WORLD_SIZE_X, WORLD_SIZE_Y };

	sf::RenderWindow window( sf::VideoMode({ WORLD_SIZE_X, WORLD_SIZE_Y }), "Swarm Simulation" );
	
	window.setFramerateLimit(FRAME_LIMIT);
	window.setPosition({0, 0});

	sf::Font font("arial.ttf");
	sf::Text cornerText(font);
	cornerText.setCharacterSize(24);
	cornerText.setFillColor(sf::Color::Yellow);


	World world{};

	std::vector<RandomWalker> walkers;

	int colorStep = 100;
	int walkerId = 0;
	for (int a = 0; a < 255; a += colorStep) {
		for (int b = 0; b < 255; b += colorStep) {
			for (int c = 0; c < 255; c += colorStep) {


				std::shared_ptr<GameLogger> logger = std::make_shared<GameLogger>("walker_" + std::to_string(walkerId));
				logger->clear();
				walkers.emplace_back( 
					sf::Color(a, b, c), 
					world, 
					logger
				);
				walkerId++;
			}
		}
	}

	std::cout << "Created " << walkers.size() << " walkers." << std::endl;

	// Clock for measuring actual framerate
	sf::Clock clock;

	float timeBuffer = 0.f;

	while ( window.isOpen() )
	{
		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() ) {
				window.close();
				// window.setPosition({0, 0});

			} else if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (mouseButtonPressed->button == sf::Mouse::Button::Right)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition( window );
					world.handleClick( mousePos );
				} else if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
					sf::Vector2i mousePos = sf::Mouse::getPosition( window );
					for (auto& w : walkers) {
						w.setTargetFromUSerInput(mousePos);
					}
				}
			} else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
					world.newRandomWalls();
				}
			}
		}

		// Delta time
		float deltaTime = clock.restart().asSeconds();
		float fps = 1.f / deltaTime;

		timeBuffer += deltaTime;
		int tickCount = 0;

		while ( timeBuffer >= TICK_DURATION ) {	
			timeBuffer -= TICK_DURATION;

			tickCount++;

			// Tick everything forward by one tick
			for ( auto& w : walkers ) {
				w.step( TICK_DURATION );
			}
		}

		window.clear();

		// window.draw( shape );
		world.display( window );

		drawWalkers( window, walkers );

		cornerText.setString( "FPS: " + std::to_string( (int)fps ) + " | Ticks: " + std::to_string( tickCount ) );
		cornerText.setPosition( { 10.f, 10.f } );
		window.draw( cornerText );

		window.display();
	}
}
