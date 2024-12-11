#!/usr/bin/env python
# Display a runtext with double-buffering.
from samplebase import SampleBase
from rgbmatrix import graphics
import time


class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)
        self.parser.add_argument("-t", "--text", help="The text to scroll on the RGB LED panel", default="How do actors kiss and not fall in love? #1604446337 7:56:00 AM What would the average single guy do if a girl falls asleep on his shoulder? #25400896 9:16:58 AM Has any couple met on Quora and fallen in love? #84000951 6:57:43 PM What do we do when everything falls apart? #49447933 5:26:25 AM How do you keep homemade burgers from falling apart? #1224633471 12:30:00 PM My mom is dying. How do I keep from falling apart? #588767159 8:59:23 PM Is it possible to fall in love online? #179982 1:03:29 AM What is life like for people who don't know how to program a computer? Do they feel something significant is missing from their lives? #6358193 7:48:46 PM Is there a secret (a how to) to listen to classical music? #9325777 2:02:48 AM How do you kiss a 12-year-old boy? I'm also 12 years old #378636078 7:35:06 AM How do you kiss? Literally, what do I do with my lips? #1179095387 7:24:13 PM How do I delete my own question from Quora #255971590 1:38:13 AM If someone were to get bit by a mouse and within 2-3 hours the bottom of their jaw hurts, is that something to be concerned about or is it just a coincidence #1897646074 1:25:26 AM What should I do with my girlfriend if we're alone? #1941349688 4:04:51 PM I have an art degree should I retrain and become a hairdresser #1950807980 5:07:49 PM Exactly what do women use Tinder for? #338905360 12:09:58 AM How can I stop thinking so I can fall asleep when I go to bed? #52477585 2:18:24 PM How do I get up if I fall? #483495128 7:17:42 PM I want to break up with my gf because she's repulsive and I didn't want to talk to her on valentine's day but I don't know what to do. What should I do? #1979531314 2:35:30 PM Are people secretly calling you a dog when they say you're barking up the wrong tree? #1976112190 7:01:38 PM Let's pretend I'm a 13-year-old boy who has a crush on a younger 13-year-old girl. She likes me back but also has a bf. Wtf do I do? #1979022235 8:26:07 AM Why does god love Americans more than people from other countries? #1973713887 10:38:49 AM How do you make your girlfriend pay for what she did? #1966774790 1:33:31 AM There's this girl that I like. Should I follow her around for 15 minutes as long as she doesn't notice me? #1978542091 1:59:50 AM What do gyms not want you to know? #514203244 2:07:43 PM What does it mean if he holds your face and looks into your eyes when you ride him? #1975363444 9:15:11 AM Have you ever felt like this thing called 'love' wasn't for you? How did you shift? #1982170856 9:11:00 PM Is it suspicious when a grown man living alone owns cats? #1969569677 12:28:37 PM Can you please make it my way to come over for a little bit and talk to me? Question deleted How can I forget every single English word that I have learnt? #1981790047 4:53:30 PM What is a spell to make someone do what you want him to do? #1974915108 2:14:25 AM I know I am not me anymore. What do I do? #1482979415 5:56:26 PM Why do I feel so much? Like every little thing, action of peoples hurts me so much. Why do I believe that no one likes me? Why am I always sad? I want to escape everything and run to some very unknown place #1984646342 4:48:18 AM Why do I always smile while talking to that girl? Is it what it's like to love someone? #1978487613 1:10:26 AM What's a better letter, K or W? #1977039999 7:36:12 AM Can someone pretend to love you for more than 10 years? #2002077160 11:14:51 AM How do I avoid atheists? #214966231 6:14:57 AM How do I forget someone whom I once loved deeply #40108107 6:27:53 PM Is it okay if I can't forgive Germans for what they did in WW2? #252110716 1:18:55 AM If I am going to die why was I born? #294134926 6:59:17 AM What would happen if I killed God? #49747216 6:35:12 AM What if I don't want to die? #296988012 4:07:54 PM What is the point of living if you are not a genius? #296346466 3:32:43 AM What if I tell you that I love you? #38234506 1:24:47 PM What if I'm god? #249342201 3:35:13 AM Is it wrong if I don't talk much? #115487581 12:09:37 PM Should I ask my girlfriend's dad If I can kiss her? #66815989 10:16:39 AM What if I don't want to move on? #239761969 10:34:18 AM I feel like I'm a horrible person. Who wants to be my friend? #2003699410 9:19:39 AM How can I just be okay with everything instead of being mad? #2003706944 9:24:20 AM Where are owls during the day? #82917446 3:28:46 AM How do I not embarrass myself in public? #755870695 10:27:48 AM Why did a girl put her legs near me while watching me in the face, as she lay on the bench and cuddled with her friends? 2:10:10 AM How do I know what is interesting in me, What happens if a girl says hi? #1991272055 7:51:14 PM Can a girl grind on another girls lap? Like is that a thing? Is it okay for my partner to repeatedly fall asleep when I'm talking to him? #543027793 11:39:14 AM Should I (24, female) confront my dad (40, male) about his new firearm obsession? It makes me extremely uncomfortable. #2003738322 9:53:16 AM Someone told me to share anything with you about which I am not feeling great. Can I share? #2005295168 5:28:34 AM Are Christian churches always safe places where you can trust everyone? #2005184955 3:48:51 AM How can I convince my neighbour to stop feeding my cat (I've already asked twice)? #2005297905 5:31:03 AM How do I tell my boss "I love you"? #2005402682 6:47:55 AM In your own opinion, what is art? #2005459270 7:32:57 AM Be honest, is believing in love a scam? If you love me and I love you, why do we hurt each other so much? #1999519455 11:04:40 PM How long after your pet dies should you wait to get another pet? #1997698784 12:59:09 AM To impress a girl, what does a young boy do? #2003787946 10:38:32 AM How can I tell if he loves me? #2005628304 9:41:53 AM What is a suave man? #2005574920 9:00:27 AM Is it weird for someone not to feel love? #1994254128 9:21:58 AM What does my girl mean when she says "oh okay nice anyway"? #2003040092 12:00:31 AM At what age does a boy start getting taller? I'm 15 and I'm only 5'4 tall. #2005644204 9:54:32 AM Why does no one love me in this world? #1999325835 8:34:03 PM Someone told me to share anything with you about which I am not feeling great. Can I share? #2005295168 5:28:34 AM Can you resent someone you love? #1992549540 12:59:50 PM How do you hurt someone by words respectfully? #2003720077 9:36:42 AM Are today's loves real? #2004450460 6:27:49 PM How do I breed guppies? #2005500158 8:04:02 AM How do you tell someone you're praying for them? #1997686984 12:48:38 AM How do you know if a guy loves you and is not using you? #2006223036 4:55:38 PM Is it safe to call my roommate my partner to my friends and family, even though she doesn't know I call her that? #2006233810 5:01:40 PM When you know what you love is a mistake, what will you do? #2005718220 10:56:42 AM When will a woman give me a hug? #1993618796 12:41:16 AM How do you know when it's time to let someone go (even if the both of you are still in love)? #2004413849 6:04:16 PM I am 6. How do I grow quickly? #2006145771 4:07:40 PM What's a word that you realized you've been misspelling the whole time? #2006183895 4:33:03 PM I loved her, but suddenly the love feeling disappeared for I don't know what reasons. What do I do? #2004298659 4:45:54 PM What should I do? I have fallen in love with a man I don't know What the heck is the point of love if it's going to always hurt? Why would anyone want to hurt? #1995870389 2:14:16 AM #2004983659 12:55:01 AM Are there any girls from the UK who want to chat? #2007619883 10:58:45 AM What does "I'm falling for you" mean? #1273684188 3:11:41 AM What did Jesus really mean when he said to follow me? #2006521781 8:04:29 PM What does it feel like when you put on a pair of ladies tights? #302515760 8:04:15 AM How do I find my way home? #710064144 9:00:01 PM How do you Control your girl? #73741971 9:27:05 PM Why do we fall out of love? #258402 12:22:51 AM How can i stop using facebook too much? #11809845 11:49:03 PM What does a man usually mean when he says keep in touch? #291758760 5:25:55 AM How do I stop controlling other people and not care about what they are doing, where and with they are going out, etc. (like my ex-GF, friends, family, etc.)? #2033690284 10:11:29 AM How do introverts start a deep conversation? #2033681143 10:03:20 AM Can I pay someone to end a relationship? #2033304241 5:12:08 AM Do cashiers in other countries sit down? #2033667495 9:51:02 AM How do introverts miss you? #2033676077 9:59:12 AM How do you make a Gemini man fall in love? #2031833276 10:57:53 AM What keeps love alive in long marriages? #2031322463 3:54:26 AM My crush dropped his book by accident and then stared at me for like 10 seconds why? #2027859560 6:18:08 AM How do I move on from a breakup? I still love her. #2032386834 5:13:01 PM Ladies, is twirling and playing with your hair really a sign of interest? #2033674195 9:57:17 AM How do you take a bath with your boyfriend? #2029921933 11:11:02 AM I love my boyfriend. Would he say it back to me? #2031252234 2:43:43 AM How do I move a relationship from online to offline smoothly? #2033280275 4:52:14 AM What should I do if I like my ex more than my boyfriend? #2023613673 12:36:39 AM What does it mean when a couple gets a dog? #2031251156 2:42:24 AM Am I wrong, or is it that my partner is abusive and manipulative? #2033704037 10:22:58 AM When you call a crisis hotline after they say hello, are you supposed to just start talking? #2031167557 1:39:58 AM Is it okay for a girl to text me you goodnight at 6pm? #2030611034 6:56:53 PM He broke up with me saying "he didn't love me anymore" and so I left without saying anything (because I didn't want to cry in front of him). Did I do right? #2033644266 9:32:56 AM Is it wrong to not love your grandpa? #2027252459 8:54:19 PM Are there any free dating sites that are specifically for poor, low-income people? #2033413112 6:38:23 AM What does "something shifts " mean? #2033712573 10:30:23 AM Can we be my friend if you don't mind? I'm on WhatsApp +27644697008. #2033721461 10:37:35 AM Should I dump my ambitionless/lazy girlfriend? #2021163331 7:10:40 PM Every time I look at the clock, it's 11:11, 3:33, 2:22, and so on. What does that mean? #2033725218 10:41:06 AM Can a puppy save a relationship? #2031251350 2:42:41 AM The only men I attract are desperate men with no standards. Am I just a low-quality female? #2033116983 2:21:52 AM How can I tell my boyfriend that I want to spend the rest of my life with him? #2032770281 9:27:01 PM If a kiss from my male boss is inappropriate, how about a peck? Is it acceptable? #2026959232 5:18:11 PM What can I do? She's leaving me for another man. #2033425292 6:47:44 AM Do narcissists eventually end up without a friend? #2035412139 8:39:08 AM Is it okay for my mom to stop talking care of me? I'm 17. #2035746320 1:07:53 PM Why did my friend ask me to have a bath with her? #2035715878 12:45:20 PM What does don't drop the soap mean? #181119916 5:33:24 PM") ,

    def run(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        font.LoadFont("../../../fonts/9x18.bdf")
        textColor = graphics.Color(255, 0, 0)  # Red color
        pos = offscreen_canvas.width
        my_text = self.args.text
        
        # Centering text vertically (assuming the font height is 13 pixels)
        font_height = 20
        canvas_height = offscreen_canvas.height
        vertical_position = (canvas_height // 2) + (font_height // 2)  # Centered vertically

        while True:
            offscreen_canvas.Clear()
            text_length = graphics.DrawText(offscreen_canvas, font, pos, vertical_position, textColor, my_text)
            pos -= 1
            if (pos + text_length < 0):
                pos = offscreen_canvas.width

            time.sleep(0.05)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)


# Main function
if __name__ == "__main__":
    run_text = RunText()
    if (not run_text.process()):
        run_text.print_help()

