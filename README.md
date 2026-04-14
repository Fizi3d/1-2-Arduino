This repo is going to contain the main code files responsible for the console/controller arduino parts of this project.

## Project description

1-2-Arduino is an arduino replica of various mini games from the Nintendo Switch Title 1-2-Switch.

## Hardware

We'll be using 3 Arduino Nanos: 1 for the main console, and the other two will host a controller.

### Console
This will have an LCD display as the main visual UI. It will also have a few buttons for selecting the game and a wireless transceiver to communicate with controllers.
The console will store the game state, prompt controllers for info when necessary, and determine who wins.

### Controllers
Each controller will essentially be a breadboard with a gyroscope, joystick, button, and wireless transceiver. Maybe even an LED.
These controllers will have information about the input they're receiving and also store relevant game information to send to the main console when prompted.
Controllers should be idle until the console starts a game, at which point the info they track depends on the selected game.

## Games

### Quick Draw
simple reaction time test with fake-outs. The console sets a timer for a random amount of time. At the end of this timer, it displays a word. If this word is "DRAW!", then the first player to flick their controller towards the other and press the button wins.
The word can also be a fake-out, like "FLAW" or "DRAG". Any input from the controllers here makes them automatically lose.

### Samurai Training
Best explained with a video. One player holds the controller over their head like a sword and the other has their hands above their head ready to clap. The clapping person needs to "catch" the sword when the other person swings. If they don't react in time, they lose. They then switch roles and repeat. Timing gets more brutal each round, and the first person to miss a round that the other person succeeds, loses.

