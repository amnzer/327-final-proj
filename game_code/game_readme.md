# Game Structure

## Game States

There are 5 states to the game

1. Intro Screen
  
    The game loads, showing a warm welcome message. The user presses a button to exit.

1. Map Select

    The game presents available maps. The user scrolls to a map and triggers gameplay.

1. Map Play

    The user is within gameplay. Audio is played, and notes are shown concurrently. The user hit notes to increase their score. Metrics concerning score, accuracy and combo are logged.

1. Map Over

    The user sees how well they did, and is given the option to return to map select or leave the game.

1. Exit Screen

    The user sees a warm exit message as they exit the game.

## Object Representations

Notes about how songs and notes are represented:

### Song Representation

Song objects store the following data:

- Path: A path to a .wav file.
- The length of the song.
- An array of Notes (defined below).

### Note Representation

Note objects hold the following data:

- Time: What beat the note falls on.
- Direction: If the note requires left, right, up, down, or neutral inputs.
- Color: The color of the note.
