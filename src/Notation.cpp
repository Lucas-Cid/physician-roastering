#include "../includes/Notation.h"
Notation::Notation(string notation) {
    this->recurrency = NULL;
    this->day = NULL;
    this->weekDay = NULL;
    this->shifts = {1, 2, 3};
    vector<char> reservedCharacteres = {'S', 'D', 'T'};
    string currentReservedCharacter = "";
    string currentNumber = "";
    for(int i = 0; i < notation.size(); i++){
        if(find(reservedCharacteres.begin(), reservedCharacteres.end(), notation[i]) != reservedCharacteres.end()){
            if(currentReservedCharacter == "" && currentNumber != ""){
                this->recurrency = stoi(currentNumber);
            } else if(currentReservedCharacter == "S"){
                this->weekDay = stoi(currentNumber);
            } else if(currentReservedCharacter == "D"){
                this->day = stoi(currentNumber);
            } else if(currentReservedCharacter == "T"){
                this->shifts = {stoi(currentNumber)};
            }

            currentReservedCharacter = notation[i];
            currentNumber = "";
        } else{
            currentNumber += notation[i];
        }
    }
    if(currentReservedCharacter == "" && currentNumber != ""){
        this->recurrency = stoi(currentNumber);
    } else if(currentReservedCharacter == "S"){
        this->weekDay = stoi(currentNumber);
    } else if(currentReservedCharacter == "D"){
        this->day = stoi(currentNumber);
    } else if(currentReservedCharacter == "T"){
        this->shifts = {stoi(currentNumber)};
    }
}

