#!/usr/bin/env python

import time
import sys
from PIL import Image
import io
import requests
from datetime import timezone, datetime
from apscheduler.schedulers.background import BackgroundScheduler

from rgbmatrix import graphics

import logging
log = logging.getLogger(__name__)

import os
path = os.path.dirname(__file__) + '/'

schedule = BackgroundScheduler(daemon=True)
schedule.start()


class SlackStatus:
    def __init__(self, offscreen_canvas, user_id, token):
        self.framerate = 1
        self.offscreen_canvas = offscreen_canvas

        self.user_id = user_id
        self.token = token
        self.status_pos = 0
        self.refresh = 10

        self.active = False
        self.status = None
        self.expiration = None
        self.icon_url = None
        self.icon = None

        self.exclude = [" • Outlook Calendar"]

        schedule.add_job(self._get_user_status)
        schedule.add_job(self._get_user_status, 'interval', seconds=self.refresh)
    
    def get_framerate(self):
        return self.framerate

    def _get_user_status(self):
        try:
            r = requests.get('https://slack.com/api/users.profile.get?user=' + self.user_id + '&pretty=1', headers={'Authorization': 'Bearer ' + self.token}, timeout=2)
            raw = r.json()['profile']

            if raw['status_text'] and raw['status_text'] != "":
                self.status = raw['status_text']
                self.active = True
                for substring in self.exclude:
                    if substring in raw['status_text']:
                        self.status = self.status.replace(substring,"")
                self.expiration = raw['status_expiration']
                if self.icon_url != raw['status_emoji_display_info'][0]['display_url']:
                    self.icon_url = raw['status_emoji_display_info'][0]['display_url']
                    self.icon = requests.get(self.icon_url, timeout=2)
            else:
                self.active = False
                self.status = "Available"
                self.expiration = 0
                self.icon_url = "https://a.slack-edge.com/production-standard-emoji-assets/14.0/apple-large/2714-fe0f.png"
                self.icon = requests.get(self.icon_url, timeout=2)
        except:
            log.warning("_get_user_status: exception occurred")

    def check_status(self):
        return self.active

    def show(self, matrix):
        self.offscreen_canvas = matrix.SwapOnVSync(self.draw())

    def draw(self):
        self.offscreen_canvas.Clear()
        font = graphics.Font()
        font.LoadFont(path + "../../fonts/5x6.bdf")
        white = graphics.Color(255, 255, 255)
        grey = graphics.Color(155, 155, 155)
        black = graphics.Color(0, 0, 0)
    
        # expiration string
        y_offset = 0
        dt = datetime.now(timezone.utc).replace(tzinfo=timezone.utc).timestamp()
        remaining = round((self.expiration - dt)/60)
        if remaining >= 0:
            expiration = "for " + str(remaining) + " mins"
            width = graphics.DrawText(self.offscreen_canvas, font, 0, 0, black, expiration)
            graphics.DrawText(self.offscreen_canvas, font, (self.offscreen_canvas.width-width)/2, 30, grey, expiration)
        else:
            y_offset = 4

        # icon
        image = Image.open(io.BytesIO(self.icon.content))
        image.thumbnail((12, 12))
        icon = image.convert('RGB')
        self.offscreen_canvas.SetImage(icon, (self.offscreen_canvas.width-12)/2, 2+y_offset)

        # status string
        width = graphics.DrawText(self.offscreen_canvas, font, 0, 0, black, self.status)
        if width > self.offscreen_canvas.width:
            self.framerate = 10
            self.status_pos -= 1
            if self.status_pos <= 0-width:
                self.status_pos = self.offscreen_canvas.width
            graphics.DrawText(self.offscreen_canvas, font, self.status_pos, 22+y_offset, white, self.status)
        else:
            self.framerate = 1
            self.status_pos = 0
            graphics.DrawText(self.offscreen_canvas, font, (self.offscreen_canvas.width-width)/2, 22+y_offset, white, self.status)

        return self.offscreen_canvas