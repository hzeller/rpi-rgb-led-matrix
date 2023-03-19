#!/usr/bin/env python

import time
import sys
from PIL import Image
import io
import requests
from datetime import timezone, datetime
from apscheduler.schedulers.background import BackgroundScheduler
from multiprocessing import Lock

from rgbmatrix import graphics

import logging
log = logging.getLogger(__name__)

import os
path = os.path.dirname(__file__) + '/'

schedule = BackgroundScheduler(daemon=True)
schedule.start()

lock = Lock()

class SlackStatus:
    def __init__(self, matrix, user_id, token):
        self.framerate = 1

        self.user_id = user_id
        self.token = token
        self.status_pos = 0
        self.refresh = 5

        self.active = False
        self.status = None
        self.expiration = None
        self.icon_url = None
        self.icon = None

        self.exclude = [" • Outlook Calendar"]

        self.matrix = matrix

        schedule.add_job(self._get_user_status)
        schedule.add_job(self._get_user_status, 'interval', seconds=self.refresh)
    
    def get_framerate(self):
        return self.framerate

    def _get_user_status(self):
        try:
            r = requests.get('https://slack.com/api/users.profile.get?user=' + self.user_id + '&pretty=1', headers={'Authorization': 'Bearer ' + self.token}, timeout=2)
            raw = r.json()['profile']

            lock.acquire()

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
        lock.release()

    def check_status(self):
        return self.active

    def show(self):
        return self.draw()

    def draw(self):        
        lock.acquire()

        offscreen_canvas = self.matrix.CreateFrameCanvas()
        _tmp_canvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        font.LoadFont(path + "../../fonts/5x6.bdf")
        white = graphics.Color(255, 255, 255)
        grey = graphics.Color(155, 155, 155)
    
        # expiration string
        y_offset = 0
        dt = datetime.now(timezone.utc).replace(tzinfo=timezone.utc).timestamp()
        remaining = round((self.expiration - dt)/60)
        if remaining >= 0:
            expiration = "for " + str(remaining) + " mins"
            width = graphics.DrawText(_tmp_canvas, font, 0, 0, grey, expiration)
            graphics.DrawText(offscreen_canvas, font, (offscreen_canvas.width-width)/2, 30, grey, expiration)
        else:
            y_offset = 4

        # icon
        image = Image.open(io.BytesIO(self.icon.content))
        image.thumbnail((12, 12))
        icon = image.convert('RGB')
        offscreen_canvas.SetImage(icon, (offscreen_canvas.width-12)/2, 2+y_offset)

        # status string
        width = graphics.DrawText(_tmp_canvas, font, 0, 0, white, self.status)
        if width > offscreen_canvas.width:
            self.framerate = 10
            self.status_pos -= 1
            if self.status_pos <= 0-width:
                self.status_pos = offscreen_canvas.width
            graphics.DrawText(offscreen_canvas, font, self.status_pos, 22+y_offset, white, self.status)
        else:
            self.framerate = 1
            self.status_pos = 0
            graphics.DrawText(offscreen_canvas, font, (offscreen_canvas.width-width)/2, 22+y_offset, white, self.status)
        
        lock.release()

        return offscreen_canvas