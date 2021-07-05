# Electrical & Computer Engineering Thesis

**"Design of Fault Detection, Isolation and Recovery in the AcubeSAT Nanosatellite"**

## Abstract

Space is not a welcoming environment; while the aerospace engineering community has managed to reliably operate thousands of satellites in orbit, CubeSats, the most popular class of nanosatellite, only have a 50% success rate. Low costs, lack of strict technical requirements and scarcity of publicly available documentation often drive up the risks for educational, scientific and commercial CubeSats. This thesis investigates a configurable and modular Fault Detection, Isolation and Recovery (FDIR) architecture that uses the ECSS Packet Utilisation Standard. This FDIR concept, along with the provided open-source software implementation, can be used by CubeSat missions to increase the reliability of their design and chances of mission success, by autonomously responding to on-board errors. The thesis also includes background information regarding CubeSat reliability, and explores the software and hardware used to implement the proposed FDIR design on the AcubeSAT mission, currently under design by students of the Aristotle University of Thessaloniki.

## This repository

### Key files

| File                  | Language | Description                                       |
|-----------------------|----------|---------------------------------------------------|
| **`main.tex`**            | English  | Source code of thesis in LaTeX (Work In Progress) |
| **`main.pdf`**            | English  | Thesis text (Work In Progress)                    |
| **`main-el.tex`**         | Greek    | Source code of thesis in LaTeX                    |
| **`main-el.pdf`**         | Greek    | Thesis text                                       |
| **`presentation-el.pdf`** | Greek    | Thesis presentation                               |

### Software repositories

| Repository | Language | Description |
|-|-|-|
| [`thesis-latex`](https://github.com/kongr45gpen/thesis-latex) | LaTeX | Complete text and LaTeX source code of the thesis |
| [`fdir-demo`](https://github.com/kongr45gpen/fdir-demo) | C++ | Microcontroller firmware |
| [`fdir-demo-yamcs`](https://github.com/kongr45gpen/fdir-demo-yamcs) | Java, XML, C++, Javascript | Ground segment software components and configuration |
| [`ecss-services`](https://gitlab.com/kongr45gpen/ecss-services/-/tree/fdir), <br>`fdir` branch | C++ | Modification of AcubeSAT's ecss-services repository.<br>Is included as a submodule in `fdir-demo` |
