#!/usr/bin/env python
# Display a runtext with double-buffering.
from samplebase import SampleBase
from rgbmatrix import graphics
import time


class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)
        self.parser.add_argument("-t", "--text", help="The text to scroll on the RGB LED panel", default="How do actors kiss and not fall in love? #1604446337 7:56:00 AM What would the average single guy do if a girl falls asleep on his shoulder? #25400896 9:16:58 AM Has any couple met on Quora and fallen in love? #84000951 6:57:43 PM What do we do when everything falls apart? #49447933 5:26:25 AM How do you keep homemade burgers from falling apart? #1224633471 12:30:00 PM My mom is dying. How do I keep from falling apart? #588767159 8:59:23 PM Is it possible to fall in love online? #179982 1:03:29 AM What is life like for people who don't know how to program a computer? Do they feel something significant is missing from their lives? #6358193 7:48:46 PM Is there a secret (a how to) to listen to classical music? #9325777 2:02:48 AM How do you kiss a 12-year-old boy? I'm also 12 years old #378636078 7:35:06 AM How do you kiss? Literally, what do I do with my lips? #1179095387 7:24:13 PM How do I delete my own question from Quora #255971590 1:38:13 AM If someone were to get bit by a mouse and within 2-3 hours the bottom of their jaw hurts, is that something to be concerned about or is it just a coincidence #1897646074 1:25:26 AM What should I do with my girlfriend if we're alone? #1941349688 4:04:51 PM I have an art degree should I retrain and become a hairdresser #1950807980 5:07:49 PM Exactly what do women use Tinder for? #338905360 12:09:58 AM How can I stop thinking so I can fall asleep when I go to bed? #52477585 2:18:24 PM How do I get up if I fall? #483495128 7:17:42 PM I want to break up with my gf because she's repulsive and I didn't want to talk to her on valentine's day but I don't know what to do. What should I do? #1979531314 2:35:30 PM Are people secretly calling you a dog when they say you're barking up the wrong tree? #1976112190 7:01:38 PM Let's pretend I'm a 13-year-old boy who has a crush on a younger 13-year-old girl. She likes me back but also has a bf. Wtf do I do? #1979022235 8:26:07 AM Why does god love Americans more than people from other countries? #1973713887 10:38:49 AM How do you make your girlfriend pay for what she did? #1966774790 1:33:31 AM There's this girl that I like. Should I follow her around for 15 minutes as long as she doesn't notice me? #1978542091 1:59:50 AM What do gyms not want you to know? #514203244 2:07:43 PM What does it mean if he holds your face and looks into your eyes when you ride him? #1975363444 9:15:11 AM Have you ever felt like this thing called 'love' wasn't for you? How did you shift? #1982170856 9:11:00 PM Is it suspicious when a grown man living alone owns cats? #1969569677 12:28:37 PM Can you please make it my way to come over for a little bit and talk to me? Question deleted How can I forget every single English word that I have learnt? #1981790047 4:53:30 PM What is a spell to make someone do what you want him to do? #1974915108 2:14:25 AM I know I am not me anymore. What do I do? #1482979415 5:56:26 PM Why do I feel so much? Like every little thing, action of peoples hurts me so much. Why do I believe that no one likes me? Why am I always sad? I want to escape everything and run to some very unknown place #1984646342 4:48:18 AM Why do I always smile while talking to that girl? Is it what it's like to love someone? #1978487613 1:10:26 AM What's a better letter, K or W? #1977039999 7:36:12 AM Can someone pretend to love you for more than 10 years? #2002077160 11:14:51 AM How do I avoid atheists? #214966231 6:14:57 AM How do I forget someone whom I once loved deeply #40108107 6:27:53 PM Is it okay if I can't forgive Germans for what they did in WW2? #252110716 1:18:55 AM If I am going to die why was I born? #294134926 6:59:17 AM What would happen if I killed God? #49747216 6:35:12 AM What if I don't want to die? #296988012 4:07:54 PM What is the point of living if you are not a genius? #296346466 3:32:43 AM What if I tell you that I love you? #38234506 1:24:47 PM What if I'm god? #249342201 3:35:13 AM Is it wrong if I don't talk much? #115487581 12:09:37 PM Should I ask my girlfriend's dad If I can kiss her? #66815989 10:16:39 AM What if I don't want to move on? #239761969 10:34:18 AM I feel like I'm a horrible person. Who wants to be my friend? #2003699410 9:19:39 AM How can I just be okay with everything instead of being mad? #2003706944 9:24:20 AM Where are owls during the day? #82917446 3:28:46 AM How do I not embarrass myself in public? #755870695 10:27:48 AM Why did a girl put her legs near me while watching me in the face, as she lay on the bench and cuddled with her friends? 2:10:10 AM How do I know what is interesting in me, What happens if a girl says hi? #1991272055 7:51:14 PM Can a girl grind on another girls lap? Like is that a thing? Is it okay for my partner to repeatedly fall asleep when I'm talking to him? #543027793 11:39:14 AM Should I (24, female) confront my dad (40, male) about his new firearm obsession? It makes me extremely uncomfortable. #2003738322 9:53:16 AM Someone told me to share anything with you about which I am not feeling great. Can I share? #2005295168 5:28:34 AM Are Christian churches always safe places where you can trust everyone? #2005184955 3:48:51 AM How can I convince my neighbour to stop feeding my cat (I've already asked twice)? #2005297905 5:31:03 AM How do I tell my boss I love you? #2005402682 6:47:55 AM In your own opinion, what is art? #2005459270 7:32:57 AM Be honest, is believing in love a scam? If you love me and I love you, why do we hurt each other so much? #1999519455 11:04:40 PM How long after your pet dies should you wait to get another pet? #1997698784 12:59:09 AM To impress a girl, what does a young boy do? #2003787946 10:38:32 AM How can I tell if he loves me? #2005628304 9:41:53 AM What is a suave man? #2005574920 9:00:27 AM Is it weird for someone not to feel love? #1994254128 9:21:58 AM What does my girl mean when she says oh okay nice anyway? #2003040092 12:00:31 AM At what age does a boy start getting taller? I'm 15 and I'm only 5'4 tall. #2005644204 9:54:32 AM Why does no one love me in this world? #1999325835 8:34:03 PM Someone told me to share anything with you about which I am not feeling great. Can I share? #2005295168 5:28:34 AM Can you resent someone you love? #1992549540 12:59:50 PM How do you hurt someone by words respectfully? #2003720077 9:36:42 AM Are today's loves real? #2004450460 6:27:49 PM How do I breed guppies? #2005500158 8:04:02 AM How do you tell someone you're praying for them? #1997686984 12:48:38 AM How do you know if a guy loves you and is not using you? #2006223036 4:55:38 PM Is it safe to call my roommate my partner to my friends and family, even though she doesn't know I call her that? #2006233810 5:01:40 PM When you know what you love is a mistake, what will you do? #2005718220 10:56:42 AM When will a woman give me a hug? #1993618796 12:41:16 AM How do you know when it's time to let someone go (even if the both of you are still in love)? #2004413849 6:04:16 PM I am 6. How do I grow quickly? #2006145771 4:07:40 PM What's a word that you realized you've been misspelling the whole time? #2006183895 4:33:03 PM I loved her, but suddenly the love feeling disappeared for I don't know what reasons. What do I do? #2004298659 4:45:54 PM What should I do? I have fallen in love with a man I don't know What the heck is the point of love if it's going to always hurt? Why would anyone want to hurt? #1995870389 2:14:16 AM #2004983659 12:55:01 AM Are there any girls from the UK who want to chat? #2007619883 10:58:45 AM What does I'm falling for you mean? #1273684188 3:11:41 AM What did Jesus really mean when he said to follow me? #2006521781 8:04:29 PM What does it feel like when you put on a pair of ladies tights? #302515760 8:04:15 AM How do I find my way home? #710064144 9:00:01 PM How do you Control your girl? #73741971 9:27:05 PM Why do we fall out of love? #258402 12:22:51 AM How can I stop using Facebook too much? #11809845 11:49:03 PM What does a man usually mean when he says keep in touch? #291758760 5:25:55 AM How do I stop controlling other people and not care about what they are doing, where and with they are going out, etc. (like my ex-GF, friends, family, etc.)? #2033690284 10:11:29 AM How do introverts start a deep conversation? #2033681143 10:03:20 AM Can I pay someone to end a relationship? #2033304241 5:12:08 AM Do cashiers in other countries sit down? #2033667495 9:51:02 AM How do introverts miss you? #2033676077 9:59:12 AM How do you make a Gemini man fall in love? #2031833276 10:57:53 AM What keeps love alive in long marriages? #2031322463 3:54:26 AM My crush dropped his book by accident and then stared at me for like 10 seconds why? #2027859560 6:18:08 AM How do I move on from a breakup? I still love her. #2032386834 5:13:01 PM Ladies, is twirling and playing with your hair really a sign of interest? #2033674195 9:57:17 AM How do you take a bath with your boyfriend? #2029921933 11:11:02 AM I love my boyfriend. Would he say it back to me? #2031252234 2:43:43 AM How do I move a relationship from online to offline smoothly? #2033280275 4:52:14 AM What should I do if I like my ex more than my boyfriend? #2023613673 12:36:39 AM What does it mean when a couple gets a dog? #2031251156 2:42:24 AM Am I wrong, or is it that my partner is abusive and manipulative? #2033704037 10:22:58 AM When you call a crisis hotline after they say hello, are you supposed to just start talking? #2031167557 1:39:58 AM Is it okay for a girl to text me you goodnight at 6pm? #2030611034 6:56:53 PM He broke up with me saying he didn't love me anymore and so I left without saying anything (because I didn't want to cry in front of him). Did I do right? #2033644266 9:32:56 AM Is it wrong to not love your grandpa? #2027252459 8:54:19 PM Are there any free dating sites that are specifically for poor, low-income people? #2033413112 6:38:23 AM What does something shifts mean? #2033712573 10:30:23 AM Can we be my friend if you don't mind? I'm on WhatsApp +27644697008. #2033721461 10:37:35 AM Should I dump my ambitionless/lazy girlfriend? #2021163331 7:10:40 PM Every time I look at the clock, it's 11:11, 3:33, 2:22, and so on. What does that mean? #2033725218 10:41:06 AM Can a puppy save a relationship? #2031251350 2:42:41 AM The only men I attract are desperate men with no standards. Am I just a low-quality female? #2033116983 2:21:52 AM How can I tell my boyfriend that I want to spend the rest of my life with him? #2032770281 9:27:01 PM If a kiss from my male boss is inappropriate, how about a peck? Is it acceptable? #2026959232 5:18:11 PM What can I do? She's leaving me for another man. #2033425292 6:47:44 AM Do narcissists eventually end up without a friend? #2035412139 8:39:08 AM Is it okay for my mom to stop talking care of me? I'm 17. #2035746320 1:07:53 PM Why did my friend ask me to have a bath with her? #2035715878 12:45:20 PM What does don't drop the soap mean? #181119916 5:33:24 PM Do all boys actually think it's funny to make a girl think you love her then leave her? #2034871386, 1:01:25 AM How do a cold hearted person fall in love? #465004801, 2:08:23 AM I accidentally cheated on my boyfriend when I walked into my best friend's house for a meal. They both know each other, so do I have hope my friend won't share? #2035773720, 1:28:47 PM Can you count on your friends? #2035422512, 8:47:49 AM How would someone be able to tell if someone is a white supremacist just by looking at them? #2035645872, 11:50:47 AM Why do some people feel like being friends with you, they are doing you a favour? #2035676077, 12:14:13 PM I am friends with benefits with my ex. What can I do to make him fall for me again? #319986401, 10:10:26 PM How do you get your wife to love you? #2027455491, 11:54:06 PM I've always had an ideal relationship/boyfriend, but my current boyfriend doesn't fit that ideal. Should I break up with him? #2035453460, 9:12:31 AM Is it grammatically correct to say I don't get no respect? #2035681894, 12:19:20 PM How many liked-minded people can we fit in one space? #2023704796, 2:01:49 AM How do I fix my boyfriend's trust issues caused by me? #2035047877, 3:44:02 AM What does it mean to be truly attracted to someone? #2035825396, 2:05:05 PM What does it mean to find an evil eye bracelet? #1438107972, 1:18:42 AM Why do you talk about your ex-husband so much? #2029937750, 11:23:46 AM What does it mean to find a true best friend? #2028809668, 7:15:16 PM How should I approach my married neighbour whom I have fallen in love with? #235054223, 5:48:34 AM In what circumstances is it ok to use another person’s toothbrush? #2034646915, 10:00:57 PM What are the stages of falling in love that are good and right? #2046231468, 8:19:07 AM I'm falling in love with my crush. Is it normal? #2046446871, 11:07:39 AM How you can fix a broken relationship? #2046455980, 11:15:20 AM What is a washed-up coward? #2046491838, 11:44:33 AM What should I do if I don't trust my wife? #2044834059, 2:50:24 PM How do I know if he really wants me or not? #2031945047, 12:18:47 PM How do you get a rich man to like you? #2046475977, 11:32:16 AM I was asked to the prom but I said no out of fear. How do I tell him I want to go with him? #2041611388, 7:24:26 PM Why do I fell in love so easily? #218000070, 9:30:12 PM Can you be lovesick without being in love? #2045671285, 12:19:31 AM Should I be disappointed because someone refused to help me? #2042424405, 7:18:46 AM I think I am going mad, what to do? please help me #17251753, 11:24:42 AM Please help me. I'm drowning! #443986914, 6:41:02 AM I feel like I’m drowning and I don't know what to do? #443986914, 6:41:02 AM Where do you guys like being kissed? #193413506, 10:05:52 PM What is this strange feeling in my chest? #642443007, 8:57:58 PM How do I read my girlfriends texts? She is cheating on me. #1961043751, 1:00:44 AM I believe I’m in love. What are the signs? #1000034611, 5:25:51 AM Is it normal to pee yourself a little? #2072912692, 4:43:12 PM I’m scared to fall in love again. What I should do? #230780641, 12:40:32 PM How do I know if he’s not worth waiting for him to fall in love with me? #2067892847, 5:10:14 PM What do you do when someone hurts you and doesn’t care? #2072928351, 4:52:12 PM How do I do convince my girlfriend it was not my fault? #2072798734, 3:28:00 PM How can I get people to stop falling in love with me? #2074104179, 8:47:45 AM I’m falling in love again. What should I do? #218644905, 5:56:02 AM How can you get along with white folks? #2075627484, 4:53:07 AM Why do you think your partner neglects you? #2077976003, 12:32:35 PM What kind of people should I avoid in life? #2069219754, 1:00:38 PM Is it possible to manipulate someone into falling in love with you? #318671642, 7:36:50 PM Is it true that love is a choice? #2077590838, 7:20:30 AM How do you get a man to keep loving you? #2068127961, 7:52:16 PM What do we need to discuss? I’m more than willing to talk. #2077745202, 9:26:03 AM Is it wrong to fall in love with a girl while I’m a girl too? #213377076, 8:19:01 PM If a person asks your dog a question, do you have to respond or just stay quiet and watch them interact? #2077957382, 12:18:16 PM What does it mean to set emotional boundaries? #2075316291, 12:05:33 AM What does it do for you when someone smiles at you? #2078003679, 12:53:48 PM Should I tell everything to my partner? #2077595253, 7:24:36 AM I got my heart broken by a hoe. What do I do? #2079738466, 9:26:43 AM What does you don’t have to be sorry mean? #1439156173, 9:46:10 PM Is the Christian life meaningless? #2074028850, 7:47:05 AM How do I know if I like my best friends boyfriend? #2078379983, 5:28:52 PM Is it hard to be nice? #2079897989, 11:51:15 AM Why does God love me so much? #2078734073, 9:13:31 PM How do fish swim against the current? #2079830291, 10:52:00 AM I don’t like it when I have to work. How do I stop this? #2072128724, 6:58:40 AM Is it okay to not feel okay? #2069149456, 12:03:55 PM Why shouldn’t you take everything seriously? #2070953078, 2:02:18 PM What would you do if I really felt broken and I just cuddled up to you and hugged you really tight? #2079842038, 11:02:49 AM Why everyone falls in love with me? #194688466, 11:11:21 AM Can I have an online friend? #2079839115, 10:59:45 AM Is it true that if an ex hates you after a breakup, they love you very much? #2073320617, 9:24:36 PM I miss my Phillip so badly and I’m too sad and tearful. Can I just hug you and hold you for a while? #2079872696, 11:29:31 AM How do you say I’m leaving? #2088516645, 10:30:35 AM What does it mean when an atheist says holy cow? #2124104406, 5:53:12 AM Why did I fall in love with someone I only saw from the back? I know what he does and likes, but I never saw his face. #2121089049, 10:51:35 AM When flatmates fall in love, are they already lovers but without an official wedding ring? #2110053301, 11:17:41 AM Were there animals in Nazi Germany? #2130397039, 4:36:59 PM How can I let my step mom fall in love with me? #1587457762, 2:12:41 AM How can I love like how Jesus loved? #2119879732, 4:34:44 PM What does it mean to be a psychic in 2022/23? #2130283963, 3:21:16 PM Why do I feel like I’ve lost the ability to fall in love with others? #2129953656, 11:03:41 AM Can you stay in a childless marriage with someone you love? #2129151934, 10:39:02 PM Is it bad to treat a woman more like a bro? #2130358406, 4:12:16 PM Is it true that if a woman you which means she likes you? #354174395, 2:50:51 AM What would a guy lead on a girl, then ask her out as a joke? #2124172569, 6:55:10 AM Why can’t we all just get along with one another? #2121742344, 7:48:09 PM How do you determine who won an argument? #2123110796, 3:53:41 PM When a man texts you on a dating site and says he is an orthopaedic surgeon working with the UN on a 2-year contract in Yemen, isn’t that a lie? #2127257834, 11:30:48 PM Why do you like to ignore others? #2126139931, 9:27:17 AM Why did my boyfriend tell another girl that he and I aren’t dating anymore? #2126696320, 4:24:34 PM I’m falling in love with my side chick. What should I do? #1054500844, 7:35:18 PM When was the last time you got a hug, excluding your family? #2130296451, 3:29:34 PM Can two people fall in love only through texting and pictures? #108196363, 8:02:42 PM Does online dating make guys bigger jerks and lazy with no concept of reality? Is it the reason why they are still available on the app? #2124005411, 4:18:44 AM Should you tell God sorry for your sin or ask him to forgive you for your sin or can you say both? #2127133057, 9:37:49 PM Why do we call our exes when we are drunk? Is there a psychological meaning to it? #102081526, 10:33:42 AM What happens if you don’t like the person you’re dating? #2123699091, 10:47:55 PM Why do you not want to give me attention? #2130301139, 3:32:44 PM What would be a better word to describe an Alpha Male since Alpha Male gets under your skin so much? #2118199341, 5:35:05 PM What are some reasons someone would rather talk to someone else than you? #2121142854, 11:38:29 AM How do I get my husband of 15 years to fall back in love with me? #2134644938, 4:27:43 AM Are you afraid to love? #30288733, 4:27:46 PM What is the meaning of peace, and how do you get it? #2137505445, 5:08:41 PM Is it my fault if a taken guy is falling for me? #2131981768, 2:22:22 PM What is the difference between I love him and I love the idea of being with him? #2130540930, 6:10:04 PM Do you want to fall in love again? #406801288, 9:13:41 AM Why am I falling in love with any girl that does eye contact with me? #2129472082, 3:50:01 AM How do narcissists sleep at night? #690544797, 3:52:35 PM What are the reasons you love someone very much, but then you block them? #2134450474, 1:36:55 AM What does next week mean on Saturday? #2134431794, 1:19:31 AM How do I become a winner when the world doesn’t even seem to give me a chance to be one? #2136169118, 11:31:36 PM How should we treat other people? #2137563656, 5:45:44 PM What are subtle signs you might be dealing with a narcissist? #604541729, 8:24:22 AM Could you describe how it feels like to be in love? #2130327819, 3:51:39 PM How do I hack my girlfriends WhatsApp without OTP? #2137176150, 1:24:01 PM I think I could find the key to immortality if I was funded. Would you want to live forever? #2134018225, 7:23:03 PM How important is spending time with your spouse? #2126745903, 4:56:35 PM Is it weird that I’m in love with this random person that I accidentally photographed? #2120762813, 5:43:09 AM Is it okay to like smelling my girlfriends sweaty socks? #2137924928, 9:59:33 PM Is it possible to shrink myself about 2 inches? If so, how? #2137926261, 10:00:51 PM If you have been hiding something for a long time, is it time to tell? #2137808412, 8:28:28 PM How do I flirt with women exactly? #2137905622, 9:43:17 PM Why does he love me but hits me from time to time but not often? #2131335837, 4:55:28 AM Is it weird to go to a birthday party with seven girls and one guy? I am the guy. #2137805580, 8:25:59 PM Why do we want people who don’t want us? #2137766848, 7:57:09 PM Is it a red flag if a boyfriend won’t show you inside his house? #2136210990, 12:06:26 AM Does art have to be intentional? #2139007244, 12:54:16 PM How do I make a girl fall in love with me unconditionally, be committed, and also control her when she needs it the most? #2138467406, 5:13:30 AM What happens if you ask someone to kiss you on the lips? #2136176190, 11:36:48 PM If a man is in love with a woman he cannot have, will he take another and pretend that it’s her? #2133038155, 5:28:42 AM Why the f*ck do women ask guys out knowing that it’s the guys role to chase? #2139002849, 12:50:54 PM What does it mean when my evil eye bracelet disappears? #1116651102, 4:21:15 PM I’ve found an evil eye bracelet, what do I do? #1438107972, 1:18:42 AM I may have fallen in love with my ex’s sister. Is that bad? #1389577190, 4:01:42 AM How do I register a word I invented? #2160796249, 6:48:45 AM What are some things that can be done to stop girls from sleeping with boys? #2154999676, 12:46:10 AM I miss the feel of a girl in my arms. Should I just go on Tinder and find a girl for right now and stop waiting for the one? #2160856552, 7:35:47 AM I ask and I ask and I ask but God isn’t doing anything. Does He hear me? Does He love me still? #2153235568, 4:44:26 AM What do people mean when they say sit on my face? #227313438, 2:40:21 PM I am depressed. How do I hide it at school? #48946345, 10:40:39 PM I feel stupid. My memory is weak. I am not intelligent. Maybe I am depressed. What should I do? #64184039, 8:06:00 PM I’m depressed, insecure and I don’t want to live. what do I do? #693039095, 12:06:50 AM How would a middle-aged woman, who no longer receives much male attention, feel about getting catcalled on the street? #2265916927, 7:00:09 PM")

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

