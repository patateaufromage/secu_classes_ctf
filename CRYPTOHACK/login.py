#!/usr/bin/python3
string = "KIUBUII FHEKT JQIA HYLUH"
alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
# iterer a travers le string
# si le char de string est dans alphabet, reperer sa valeur dans alphabet, et prendre la valeur suivante
# si le char a index j == le dernier index de alphabet, alors sa valeur est la premiere de alphabet
length = len(string)

def decoder(cypher):
    new_string = ""
    for j in range(0, length):
        if string[j] in alphabet:

        else:
            new_string += " "
    return new_string

for i in range(0, len(string)):
    print(decoder(i))
