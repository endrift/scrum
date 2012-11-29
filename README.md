<img src="http://jpfau.github.com/scrum/resources/title.png">

It's past 3 AM on Thursday morning (or a Wednesday night, if you're going to keep lying to yourself), and there's a Friday deadline bearing down on you. You have to finish coding your GAME for the GitHub Game Off 2012 challenge, but you're not quite done yet and you haven't had enough sleep….

You take another sip from your PINT GLASS OF COLD COFFEE and your eyes go funny. The screen blurs and the lines of code all blend together. The combination of caffeine overdosing and the abject lack of sleep has definitely affected your BRAIN in the strangest of ways.

The screen flickers off.

It is pitch black. You are likely to be eaten by a GRUE.

There are no GRUES around, however. Only something more sinister…. The screen flashes and you realize you have been transported inside of a BAD PROGRAMMING METAPHOR!

Now you must fight your way through COLORFUL LINES OF CODE and EXPLODING BUGS to survive through the night!

This is SCRUM. Enjoy your stay.

# What is Scrum?

<img src="http://jpfau.github.com/scrum/resources/title-screen.png">

Scrum is an entry for the GitHub Game Off 2012 competition. It's not a text adventure, however: it's a puzzle game based on a simple premise: How can I misrepresent programming as a game without making it some arbitrary minigame that may or may not involve Pipe Mania.

The initial brainstorming session led me to a natural conclusion: a puzzle game, but line-based. While it superficially similar to Tetris, the gameplay is very different. See the tutorial section for a rundown on the gameplay.

The web version of the game can be found on [GitHub](http://jpfau.github.com/scrum). But there's a twist: while the GitHub Game Off rules state that the game must run in a web browser, it does not state *how*, and in accordance with this, the game is programmed in the most arcane manner possible: it's a Game Boy Advance game.

## Wait, what?

Scrum was written from scratch, using only DevKitPRO and associated libraries, for the Game Boy Advance. A freely-distributable ROM is [available for download](http://jpfau.github.com/scrum/resources/scrum.gba) and works (relatively) well in most GBA emulators and even on real GBA hardware, if you have a way of running them.

## But, how?

The GitHub Game Off rules state that the entries can use any libraries or runtimes they like, e.g. Unity3D or Flash, so long as they're embedded in the browser. So what better than a pure JavaScript/HTML5 Game Boy Advance emulator like [GBA.js](http://jpfau.github.com/gbajs/), which was developed primarily between July and October of 2012?

## So, why?

The [IOCCC](http://www.ioccc.org) has a Best/Worst Abuse of the Rules category, which I've always admired. I'm hoping maybe the GitHub Game Off will have one, too. It was an interesting challenge, and given that I'd just recently written GBA.js, the platform was fresh on my mind and I figured I could probably pretty easily write a game for it, with all I'd learned from the process. Turns out it was more than enough to write a game for the GBA. Plus, it's really damn cool to be able to take a GBA out of my pocket, turn it on, point to the screen and say, "I wrote this!"

## Hold on a second…

JavaScript is, unfortunately, a rather high-level language. As a result, it can be slow. GBA.js is no exception, so Scrum can run slowly in some browsers or on some platforms. You will generally need good hardware. A Sandy Bridge i5 or better is recommended hardware, and the browser should be either Safari 6.0 or Chrome 22+. Firefox and Opera also work, but are much slower and have no sound. IE10 also apparently works, but is highly un-recommended.

# Gameplay

*TODO*

# Source

The source code for the game is available on [GitHub](http://github.com/jpfau/scrum/) under the BSD 2-clause license. Compilation requires installing DevKitARM and libgba. There are no further dependencies, and building should work out-of-the-box using GNU Make (make on Linux or Mac, gmake on BSDs).

Scrum is © 2012 Jeffrey Pfau