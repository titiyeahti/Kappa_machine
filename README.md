# THE Kappa Machine

The goal of the project was to simulate a turing machine using the graph rewriting language kappa.

## Generic Kappa model

Our solution to the problem was to construct a real turing machine (an automaton connected with a tape of memory) inside kappa.
Then the rules would be identical for every different input turing machine and initial memory state.

### Agents

To construct our turing machine we relied on 4 different agents:
- C, cell of the tape :
	- Contains TRUE FALSE or SEP (the separator);
	- Connected with at most two others cells (double linked list);
	- The lecture head is connected to the current cell during the computations.
- H, lecture head :
	- Connected with the current cell of the tape and the actual state / transition in the automaton;
	- It is the central agent of the model.
- S, state of the automaton
	- **CONSTANT** number of previous transition (actually 6);
	- 3 outgoing transition (one for each symbol (TRUE FALSE SEP);
	- The current state is connected wit the head.
- T, transition in the autmaton
	- Between 2 states
	- Contains the instruction to be done when the head arrives on it : what to write, where to go.

### Code
The code of the model is stored in two distinct files:
- `model/turing_model.ka`, description of the different agents;
- `model/turing_rules.ka`, exhaustive set of rules compatible with every turing machine.

## Compiling to kappa for the inital state

We also wrote a parser to compile a turing machine description (writen in a syntax descibed just below) to a kappa "%init:" clause.

### Syntax
This describe the syntax of the input files for the parser :
```
N M  // N = Number of states, M = Size of initial memory 
{t f s}^M
[0-n] {t f s} {l r n}  // transition with f (FALSE) from 0
[0-n] {t f s} {l r n}  // transition with t (TRUE) from 0
[0-n] {t f s} {l r n}  // transition with s (SEP) from 0
...
[0-n] {t f s} {l r n}  // transition with f (FALSE) from N-1
[0-n] {t f s} {l r n}  // transition with t (TRUE) from N-1
[0-n] {t f s} {l r n}  // transition with s (SEP) from N-1
```
This file contains 3N+2 lines. The lines after the third are describing a transition the following way : 
- destination, i < n;
- value to write, {t f s} TRUE or FALSE or SEP;
- move, {l r n} LEFT or RIGHT or NO\_MOVE.

### Usage

```bash
gcc parser.c
./a.out input_file output_file
```
Be careful to respect the following constraints :
- The `input_file` should respect the syntax described above AND must exist;
- The directory containing `output_file` should exist although the file itself will be created during the execution.

Then you should load `model/turing_model.ka`, `model/turing_rules.ka` and `output_file` to your kappa engine to run the turing machine.
