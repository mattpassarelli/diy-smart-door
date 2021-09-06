# DIY Smart Door
This is, ideally, going to be a relatively universal smart door/lock adapter made from an ESP8266/ESP32 and one-two MG995 servos. More on what each servo does in the next section. This would then be connected to my home WiFi network where I can integrate it into Home Assistant for any kind of automation I would want to add, like opening the door for my friends without the need of me getting up from the couch :). 

## Design

Using an ESP8266 specifically (as I save my ESP32's for other projects), it's going to have 2 servos wired to it to open my door. If you choose the copy this project, I would guarantee you that like >90% of people only need 1 servo. My current apartment's door is this weird style I've never seen where you *have* to use the key to open the door, regardless of whether or not anything is locked. Meaning, I only really have a deadbolt, and not a latch lock that I can engage and disengage. So even if I left the deadbolt unlocked all the time, you would need a key to be able to turn my door's latch and handle to enter my apartment.

So, now the mechanical design ideas; There are a few 3D models in this repo that should be fairly universal. Right now, both planned and ready are:
- the housing for the servo that will be secured to the door
- the latch cover that goes over my deadbolt and fits one of the servo horns
- a housing for the arduino and a battery and charger
- and the servo housing for the door handle portion. 

That list might be updated as I add or remove parts from my design.

There are also 2-3 buttons for this system: 

- a lock button
- an unlock button
- an unlock iff locked, then turn handle button

#### Servo One
The first servo will be secured in housing that is then, most likely, command stripped to the door. 1) because I don't want to damage the door by screwing into it, and 2) if something were to happen where I need to immediately exit my home, I could rip the whole thing off in one swoop and then repaint easily if anything got damaged. 

This servo is connected to the deadbolt by an adapter that sits on top of the bolt and has a horn glued into it. The first motion of the whole overarching code will be to turn this servo 150° clockwise to unlock, then the same degrees counter clockwise to close (the number will change based on your deadbolt, so be sure to measure). This will have a timer running, so that if the lock is unlocked for, say 10 minutes, it will automatically lock it, giving me some peace of mind. I do also plan on integrating some automation logic into this with Home Assistant that might mitigate the need for this, but either will do the trick. 

Right now, this is controlled by a button on my breadboard, but I have plans to automate this with Home Assistant later.

#### Servo Two, the door "opener"
This is the one that not everyone will need, but you may need something like Servo One attached to your latch lock if you have a "normal" door. This one will be to the side of the deadbolt, positioned directly above the very edge of the door handle. With a stiff rod attached to a horn, maybe like a hanger wire or wooden dowel, and the other end on a free moving pivot on the door handle, this servo will turn *after* the deadbolt is unlocked and if **and only if** the deadbolt is fully unlocked. This is will the last part of my "hardware portion" of this project, so this might vary greatly by the time I've figured out how best to do this.

This one will be a third button that unlocks the deadbolt if it isn't already unlocked, then turn the servo, "opening" the door for someone to be able to push open the door. Then it will release the handle after a very short amount of time, say 5 seconds, so that the door can close on it's own and lock itself like normal (I have one of those hydraulic arms attached the top of the door that automatically closes it and was there when I signed my current apartment).

## Power

Nothing too special here, but will probably need some tuning. I have a 3.7V 4400mAh battery laying around that I intend to use. I grabbed [this USB Type C charging circuit here from Adafruit](https://www.adafruit.com/product/4410) and will use that to charge the whole circuit. While power draw will vary wildly based on how often the locks are opened and closed, I expect this to last a solid amount of time between charges.


## Write Code

As a small side note, it turns out that PlatformIO handles the servo library differently (or wrong) so it caused my servos to turn really slowly and not go the fully distance that I had coded. So instead of trying to debug it, I have an Arduino IDE .ino file of `main.cpp` so that I can use Arduino's IDE to write the code and have it work as intended.