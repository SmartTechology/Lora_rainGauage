# Lora_rainGauage
<p> Lora rainGuage is a project for a rain gauge running on the LoRa network. The criteria is that it's cheaper than the conventional rain gauge and it runs for a long time on batteries. 

## Set-up

<p>Setting it up is really easy.
It's a pull up system so all you need to do is to plug in of the jumper cables in to one side to the ground. After that you plug another jumper in the diagonally to the other one. Plug that one in the A2 port and your done!</p>
<p> A3 can be used for the other sensor, This one is easy to write because it doenst need to wake up the Nexus. </p> 
<p>The pinout is available <a href="https://webshop.ideetron.nl/Nexus">here<a></p>

## Code explanation

![](https://github.com/SmartTechology/Lora_rainGuage/blob/master/readme/interrupt.PNG)

### What is a interrupt
<p> An interrupt is information that changes the normal process. A good example is typing, the OS can't predict what your going to type so every time you press a key it creates a interrupt.

### In this code 
<p> In this code we create an interupt that when the button is pressed it interrupts the sleep protocall.

![](https://github.com/SmartTechology/Lora_rainGuage/blob/master/readme/loop.PNG)

## Loop
The loop is the code that takes the button and when it's pressed increases the value and then sets the variable buttonPressed to false so it goes back to sleep.

![](https://github.com/SmartTechology/Lora_rainGuage/blob/master/readme/rainRead.PNG)

## Time to calculate
In this piece of code we take our rainCounter value and times it by 0.15 becuase the mill collects 0,15mm per bucket. After that you times it by 10 for the TTN Network.

## Sending data to the TTN

![](https://github.com/SmartTechology/Lora_rainGuage/blob/master/readme/sendData.PNG)
<p>At the end of the code to send data to TTN you need to add this piece of code "Lora.TX.Data[] and the number is the byte number. It starts a 0 and ends where you want it to end. Also if you send a 16 bit integer you have to split it into 2 beacuse it can only send 8 bytes per message. To do this you split it into high and low byte. </p>
<hr>

# Disclamer
<p> Most of the code is written by IDEETRON and I thank them for there hard work. All I have done is edit it to a rain gauge. So many thanks again to the guys at IDEETRON. 
  
Remember have fun <br>
Kind Regards <br>
Luke
<hr>

