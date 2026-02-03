## Abstract

**Title**: The Implementation of a Custom Linux Shell

**Keywords**: Linux, User Space, Tokenisation, Parsing

**Type**: Individual Project

**Duration**: August 2023 – Sep 2023

**Course Name**: [COMP3300](https://programsandcourses.anu.edu.au/2023/course/comp3300) – Operating System Implementation

**Course Final Mark**: 83 / 100

**Project Weight**: [15%](https://programsandcourses.anu.edu.au/2023/course/COMP3300/Second%20Semester/5638) of COMP1130

**Project Mark**: 98 / 100

**Deliverables**: [report](https://github.com/glowing-sea/custom-linux-shell/blob/main/deliverables/report.pdf), [code](https://github.com/glowing-sea/custom-linux-shell)

**Description**:

- This project has implemented a customised Linux shell in C capable of running programs in parallel or in a pipeline and processing interactive or batched commands that are not necessarily well-formatted.
- The shell consists of a tokeniser and a parser that decompose complex input commands step by step based on the precedence of the operators "|", ">", and "&".
- The project has also implemented the list data structure from a raw array, capable of automatically growing and allocating/deallocating memory from the heap.
- The project indicates the need to upgrade from a hard-coded Precedence-Based Linear Parser to a Recursive Parser, such as an LL(k) parser, if the input commands contain brackets, that is, changing from the current Finite Language Grammar to a Context-Free Grammar.
