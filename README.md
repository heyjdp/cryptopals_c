# Cryptopals in C Code

![CI](https://github.com/heyjdp/cryptopals_c/actions/workflows/ci.yml/badge.svg)

Solve the **Cryptopals Crypto Challenges** using **pure C**.  

- This repository contains my implementations, notes, and explanations for each challenge set, focused on clarity, correctness, and learning modern applied cryptography from first principles.

Basically a quiet Saturday and I am in need of a challenge, so I am speedrunning the Cryptopals challenges in C code, just for fun.

---

## Overview

The **Cryptopals Challenges** are a well-known set of practical cryptography exercises covering topics like:

- Encoding and decoding  
- XOR ciphers  
- Block ciphers (AES)  
- Padding oracles  
- CBC, CTR, stream attacks  
- Diffie–Hellman, RSA, and more  

This repo is my attempt to solve them **from scratch in C**, without relying on high-level crypto libraries unless the challenge's intent allows it.

The goal is to deeply understand how the primitives work internally.

---

## Requirements

- **C99 or newer**
- **Make**
- **doxygen and pdflatex/texlive environment for docs**

---

## Building & Running

Common combinations of the below:

```bash
make clean
make
make test
make docs
```

---

## Security Disclaimer

- This code is for my own amusement and educational purposes only.
- Do not use it in production, security-critical, or cryptographic applications.

---

## References

[Cryptopals Challenges](https://cryptopals.com/)
