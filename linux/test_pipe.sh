#!/bin/bash

echo -n "Введите числа через пробел: "
read user_input

echo "$user_input" | ./M | ./A | ./P | ./S
