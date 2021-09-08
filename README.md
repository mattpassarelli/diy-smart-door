# DIY Smart Door
This is, ideally, going to be a relatively universal smart door/lock adapter made from an ESP8266/ESP32 and one-two MG995 servos. More on what each servo does in the next section. This would then be connected to my home WiFi network where I can integrate it into Home Assistant for any kind of automation I would want to add, like opening the door for my friends without the need of me getting up from the couch :). 

## Design

Using an ESP8266 specifically (as I save my ESP32's for other projects), it's going to have 2 servos wired to it to open my door. If you choose the copy this project, I would guarantee you that like >90% of people only need 1 servo. ~~My current apartment's door is this weird style I've never seen where you *have* to use the key to open the door, regardless of whether or not anything is locked. Meaning, I only really have a deadbolt, and not a latch lock that I can engage and disengage. So even if I left the deadbolt unlocked all the time, you would need a key to be able to turn my door's latch and handle to enter my apartment.~~ I have what's called a Store Room style latch. I'm going to work off the assumption that my apartment will switch it out for me (going off the reasons that I risk getting locked out frequently, which technically isn't wrong); so we'll only need the one main servo for now*.

So, now the mechanical design ideas; There are a few 3D models in this repo that should be fairly universal. Right now, both planned and ready are:
- the housing for the servo that will be secured to the door over the deadbolt
- the latch cover that goes over my deadbolt and fits one of the servo horns
- a housing for the arduino and a battery and charger
- and the servo housing for the door handle portion*

That list might be updated as I add or remove parts from my design.

There are also 2-3 buttons for this system: 

- a lock button
- an unlock button
- an unlock iff locked, then turn handle button*

### Servo One
The first servo will be secured in housing that is then, most likely, command stripped to the door. 1) because I don't want to damage the door by screwing into it, and 2) if something were to happen where I need to immediately exit my home, I could rip the whole thing off in one swoop and then repaint easily if anything got damaged. 

This servo is connected to the deadbolt by an adapter that sits on top of the bolt and has a horn glued into it. The first motion of the whole overarching code will be to turn this servo 150° clockwise to unlock, then the same degrees counter clockwise to close (the number will change based on your deadbolt, so be sure to measure). This will have a timer running, so that if the lock is unlocked for, say 10 minutes, it will automatically lock it, giving me some peace of mind. I do also plan on integrating some automation logic into this with Home Assistant that might mitigate the need for this, but either will do the trick. 

Right now, this is controlled by a button on my breadboard, but I have plans to automate this with Home Assistant later.

### Servo Two, the door "opener"*
This is the one that not everyone will need, but you may need something like Servo One attached to your latch lock if you have a "normal" door. This one will be to the side of the deadbolt, positioned directly above the very edge of the door handle. With a stiff rod attached to a horn, maybe like a hanger wire or wooden dowel, and the other end on a free moving pivot on the door handle, this servo will turn *after* the deadbolt is unlocked and if **and only if** the deadbolt is fully unlocked. This is will the last part of my "hardware portion" of this project, so this might vary greatly by the time I've figured out how best to do this.

This one will be a third button that unlocks the deadbolt if it isn't already unlocked, then turn the servo, "opening" the door for someone to be able to push open the door. Then it will release the handle after a very short amount of time, say 5 seconds, so that the door can close on it's own and lock itself like normal (I have one of those hydraulic arms attached the top of the door that automatically closes it and was there when I signed my current apartment).

*Right now, I'm trying to get my door handle's replaced by the apartment (so they still have legal, master access when needed). So this second servo will ideally be unneeded (or possible replaced by one that turns the little latch on a typical "privacy" style door latch).*

## Power

Nothing too special here, but will probably need some tuning. I have a 3.7V 4400mAh battery laying around that I intend to use. I grabbed [this USB Type C charging circuit here from Adafruit](https://www.adafruit.com/product/4410) and will use that to charge the whole circuit. While power draw will vary wildly based on how often the locks are opened and closed, I expect this to last a solid amount of time between charges.

[This Adafruit forum page](https://forums.adafruit.com/viewtopic.php?f=8&t=61747#wrap), specifically the comments by `adafruit_support_mike` were super helpful in making a really simple battery wiring circuit for this. Basically just use the picture OP made, replacing the joint wires with the comments made by Mike. 


## Write Code

As a small side note, it turns out that PlatformIO handles the servo library differently (or wrong) so it caused my servos to turn really slowly and not go the full distance that I had coded. So instead of trying to debug it, I have an Arduino IDE .ino file of `main.cpp` so that I can use Arduino's IDE to write the code and have it work as intended.

## Home Assistant and Google Home Integrations

This is the really fun stuff here. Let's start with getting it integrated into HA, then we'll do Google.

### Home Assistant

Initially, I wanted to use ESPHome, a system built for Home Assistant that (from what I understand), writes the code for you to your ESP based boards, to do specific tasks. Unfortunately, Servos aren't officially support by HA and the workarounds they offer didn't seem like they'd offer enough versatility to do what I needed. So in the main code here, we setup a really *really* simple web server on the board, along with a static IP so it never changes. This web server takes simple GET requests from any client and then performs the respecitive command that the function calls. 

e.g

```
void openLockRemotely() {
    if (isUnlocked) {
        httpRestServer.send(500, "text/html", "Deadbolt is already unlocked");
    }
    openLock();
    httpRestServer.send(200, "text/html", "Deadbolt is now unlocked");
}

void restServerRouting() {
    httpRestServer.on("/", HTTP_GET, []() {
        httpRestServer.send(200, F("text/html"),
                            F("Welcome to the Smart Door!"));
    });
    httpRestServer.on(F("/openLock"), HTTP_GET, openLockRemotely);
    httpRestServer.on(F("/closeLock"), HTTP_GET, closeLockRemotely);
    httpRestServer.on(F("/getLockStatus"), HTTP_GET, getLockStatus);
}
```
The rest server is listening on the current 3 endpoints, `openLock closeLock & getLockStatus`. When you go to `http://<BOARD'S_IP_ADDRESS>:8080/openlock` it will call `openLockRemotely` which is just a wrapper function for `openLock` and then returns a really simple HTTP 200 code so we know that the function completed. 

`getLockStatus` returns the value of the boolen `isUnlocked` and then this is displayed in HA for us to see.

What I did to make these call-able from HA is to use the REST integration built into HA that is then called from different automations and scripts. In your `configuration.yaml` you'll want to add something like the following, changing the url and/or names to suite your case:

```
rest_command:
  open_door_lock:
    url: "http://192.168.0.177:8080/openLock"
    method: get
    content_type: "application/x-www-form-urlencoded"
  close_door_lock:
    url: "http://192.168.0.177:8080/closeLock"
    method: get
    content_type: "application/x-www-form-urlencoded"
```

We'll then call these rest commands from an automation, like the one used to unlock the deadbolt:
```
alias: Unlock Deadbolt
description: ''
trigger: []
condition: []
action:
  - service: rest_command.open_door_lock
mode: single
```
As a note: I greatly prefer doing automation and script making from the Home Assistant GUI, these YAML file examples are mostly just for reference, but you could copy and paste them into the YAML editors instead.

I also use a script to call the REST command, which is used in some of the Apple Watch complications I wanted to use:
```
sequence:
  - service: rest_command.open_door_lock
mode: single
alias: Unlock Front Door
icon: mdi:lock-outline
```

From here, you can setup the LoveLace UI however you'd prefer to in order to make use of these automations. If you'd like to copy what I use, this is the YAML code for it:
```
type: vertical-stack
cards:
  - type: glance
    entities:
      - entity: sensor.deadbolt_status
    title: Front Door
    show_icon: true
    state_color: false
  - type: horizontal-stack
    cards:
      - entity: automation.lock_deadbolt
        tap_action:
          action: call-service
          service: rest_command.close_door_lock
          service_data: {}
          target: {}
        type: button
        icon: mdi:lock-outline
        icon_height: 48px
        hold_action:
          action: none
      - type: button
        tap_action:
          action: call-service
          service: rest_command.open_door_lock
          service_data: {}
          target: {}
        entity: automation.unlock_deadbolt
        icon_height: 48px
        hold_action:
          action: none
        icon: mdi:lock-open-outline
```

And it looks like this [![UI Exmaple]](https://imgur.com/rwhDQag)

### Google Assistant Integrations

First step here is to integrate Google with HA. There are a few ways to do this, and you can find documentation on it on the HA docs website, as it's too much to cover in a README here. I actually wound up subscribing to Home Assistant Cloud as it's only $5/month and the intregation with Google (and Alexa) is essentially one click. 

Once you have Google integrated, you'll want to expose the scripts to Google from HA. You can do this from `Configuration >> Home Assistant Cloud >> Google Assistant` and then select manage entities. Make sure that the two scripts we made for Locking and Unlocking the deadbolt are checked off. On a side note, I'd recommend un-exposing anything that's already integrated into GA, as it caused duplicates for everything I already had set up, which was annoying to resolve before learning of this.  

With the scripts and other entities exposed, bo back and click the `Sync Entities To Google` button and then open Google Assistant/Google Home on your phone or tablet (or chomebook maybe?) and tell it to "Sync my devices." From here, any entities you synced will become visible in the "Unassigned" Room, but not scripts! Scripts are usable, but since they aren't entities, they aren't assigned an object or room in Google Home. You can still say "Hey/Okay Google, *run* <SCRIPT_NAME>" and it'll execute. 

If, like me you'd prefer a more natural language way of doing this, set up a routine in Google Home. So, for example I set one up that activates on the phrase "Unlock the front Door" and this triggers the script, which you can set from `Add Action >> Adjust Home Devices >> Adjust Scenes >> <YOUR_SCRIPT_NAME>`.

However you set it up, rountine or not, this will trigger the Home Assistant Script we wrote, which triggers the REST Command, which calls the arduino that then activates the corresponding function to lock or unlock the deadbolt. Sounds convoluted, but this chain is probably how most commerical smart door locks operate, give or take the HA integrations. 
