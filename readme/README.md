# Lora_rainGauage

## Set-up

<p>Setting it up is really easy.
It's a pull up system so all you need to do is to plug in of the jumper cables in to one side to the ground. After that you plug another jumper in the diagonally to the other one. Plug that one in the A2 port and your done!</p>
<p> A1 can be used for the other sensor, This one is easy to write because it doenst need to wake up the Nexus. 

## Code explanation

![](https://github.com/SmartTechology/Lora_rainGuage/blob/master/readme/interrupt.PNG)

### What is a interrupt
<p> An interrupt is information that changes the normal process. A good example is typing, the OS can't predict what your going to type so every time you press a key it creates a interrupt.

### In this code 
<p> In this code we create an interupt that when the button is pressed it interrupts the sleep protocall and takes the variable rainCounter and encreases the value by 1
