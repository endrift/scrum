![Logo](http://endrift.github.com/scrum/resources/title.png)

It's past 3 AM on Thursday morning (or a Wednesday night, if you're going to keep lying to yourself), and there's a Friday deadline bearing down on you. You have to finish coding your GAME for the GitHub Game Off 2012 challenge, but you're not quite done yet and you haven't had enough sleep….

A strange thought occurs to you. What happens if you make the game for the GAME BOY ADVANCE instead of a WEB BROWSER like the rules say? All you need to do is write a GAME BOY ADVANCE EMULATOR IN JAVASCRIPT. You wonder if someone's already done that, though. It seems like someone must have by now.

You take another sip from your PINT GLASS OF COLD COFFEE and your eyes go funny. The screen blurs and the lines of code all blend together. The combination of caffeine overdosing and the abject lack of sleep has definitely affected your BRAIN in the strangest of ways.

The screen flickers off.

It is pitch black. You are likely to be eaten by a GRUE.

There are no GRUES around, however. Only something more sinister…. You hear a noise coming from your CLOSET. You open the door and discover that your loyal GAME BOY ADVANCE has somehow turned itself on and is glowing the title screen of the GAME you have been writing for the past month. The screen flashes and you realize you have been transported inside of a BAD PROGRAMMING METAPHOR!

Now you must fight your way through COLORFUL LINES OF CODE and EXPLODING BUGS to survive through the night!

This is SCRUM. Enjoy your stay.

# What is Scrum?

![Title screen](http://endrift.github.com/scrum/resources/title-screen.png)

Scrum is an entry for the GitHub Game Off 2012 competition. It's not a text adventure, however. It's a puzzle game based on a simple premise: How can I misrepresent programming as a game without making it some arbitrary minigame that may or may not involve Pipe Mania?

The initial brainstorming session led me to a natural conclusion: a puzzle game, but line-based. While it superficially similar to Tetris, the gameplay is very different. See the tutorial section for a rundown on the gameplay.

The web version of the game can be found on [GitHub](http://endrift.github.com/scrum). But there's a twist: while the GitHub Game Off rules state that the game must run in a web browser, it does not state *how*, and in accordance with this, the game is programmed in the most arcane manner possible: it's a Game Boy Advance game running on a JavaScript Game Boy Advance emulator.

### Wait, what?

Scrum was written from scratch, using only DevKitPRO and associated libraries, for the Game Boy Advance. A freely-distributable ROM is [available for download](http://endrift.github.com/scrum/resources/scrum.gba) and works (relatively) well in most GBA emulators and even on real GBA hardware, if you have a way of running them.

### But, how?

The GitHub Game Off rules state that the entries can use any libraries or runtimes they like, e.g. Unity3D or Flash, so long as they're embedded in the browser. So what better than a pure JavaScript/HTML5 Game Boy Advance emulator like [GBA.js](http://endrift.github.com/gbajs/), which was developed primarily between July and October of 2012?

### So, why?

The [IOCCC](http://www.ioccc.org) has a Best/Worst Abuse of the Rules category, which I've always admired. I'm hoping maybe the GitHub Game Off will have one, too. It was an interesting challenge, and given that I'd just recently written GBA.js, the platform was fresh on my mind and I figured I could probably pretty easily write a game for it, with all I'd learned from the process. Turns out it was more than enough to write a game for the GBA. Plus, it's really damn cool to be able to take a GBA out of my pocket, turn it on, point to the screen and say, "I wrote this!"

### Hold on a second…

JavaScript is, unfortunately, a rather high-level language. As a result, it can be slow. GBA.js is no exception, so Scrum can run slowly in some browsers or on some platforms. You will generally need good hardware. A Sandy Bridge i5 or better is recommended hardware, and the browser should be either Safari 6.0 or Chrome 22+. Firefox and Opera also work, but are much slower and have no sound. IE10 also apparently works, but is highly un-recommended.

# Gameplay

![Begin Programming!](http://endrift.github.com/scrum/resources/begin.png)

When you start the game, you're dumped into a "development environment" as it clones the repository so you can begin your very long night of coding. The board is populated and then you can begin. The main gameplay consists of trying to complete lines of code in your program.

Each line contains blocks of four colors. You can place new blocks into the line of your choosing by pressing `Z` (or `A` if you're on a GBA), and when you fill up the line all the way up to the delimited column, the portion of that line that matches the last color on the line is cleared.

![Clearing a portion of a line](http://endrift.github.com/scrum/resources/clearing.png)

You can clear an whole line by making the line entirely one color. Be careful, though! The more lines you clear, the harder the game gets. You only get a limited amount of time to decide which line a block should go on, and the more lines you have, the shorter that duration gets.

![Clearing an entire line](http://endrift.github.com/scrum/resources/clear-line.png)

If, however, you go over the line, you've made one bug for each column over you go. Furthermore, if you place a block of a mismatched color onto an existing line, you also get a bug. Watch your bug count! If it goes too high, you need to start debugging and will be automatically kicked into the debugger!

![Clearing a portion of a line](http://endrift.github.com/scrum/resources/going-over.png)

If your bug counter isn't greyed out, you can voluntarily enter debugging mode by pressing `X` (`B` if you're on a GBA). You can manually exit the debugger when you've patched enough bugs by pressing `X` again, or if you meet your quota, you'll get kicked out of the debugger automatically. If you fail at debugging, however, it's GAME OVER.

But what's a good development environment without a good revision control system behind it? You can create a local branch by hitting `A` (or `L` on a GBA). Each branch has its own score, line count and bug count, and you only get one local branch at a time, so use it wisely. Once you have a local branch, you can switch between your local branch and the master branch by hitting `A` again. Then, once you want to merge your changes back into master, or if you want to get rid of the local branch, hit `S` (or `R` on a GBA) and it will discard the branch you're not currently on. Unfortuantely, it seems that the version of git you're using isn't very good at resolving merge conflicts of colored blocks, so any changes you've made on the other branch are lost when you merge.

When selecting a difficulty, you can press `\` (or `Select` on a GBA) to view high scores for that difficulty. The game can be paused by pressing `Enter` (or `Start` on a GBA).

# Source

The source code for the game is available on [GitHub](http://github.com/endrift/scrum/) under the BSD 2-clause license. Compilation requires installing DevKitARM and libgba. There are no further dependencies, and building should work out-of-the-box using GNU Make (make on Linux or Mac, gmake on BSDs).

Scrum is © 2012 Jeffrey Pfau
