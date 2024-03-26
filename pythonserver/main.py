import os
from dotenv import load_dotenv
import requests
from icalendar import Calendar

from typing import Optional
from fastapi import FastAPI
from fastapi.responses import JSONResponse

load_dotenv()

# Your iCal calendar URL
ICAL_URL = os.getenv('ICAL_URL')

app = FastAPI()

@app.get("/")
async def root():
    return {"message": "Hello World"}

@app.get("/events")
async def get_events(max_results: Optional[int] = 10):
    try:
        # Fetch calendar data from the URL
        response = requests.get(ICAL_URL)
        response.raise_for_status()

        # Parse the calendar data
        calendar = Calendar.from_ical(response.text)

        extracted_events = []
        for event in calendar.walk('vevent')[:max_results]:
            summary = event.get('summary')
            start_time = event.get('dtstart').dt.strftime('%Y-%m-%d %H:%M:%S')
            end_time = event.get('dtend').dt.strftime('%Y-%m-%d %H:%M:%S')
            extracted_events.append({
                'summary': str(summary),
                'start_time': str(start_time),
                'end_time': str(end_time)
            })

        return extracted_events

        # Extract necessary information from events
        # extracted_events = []
        # for event in calendar.walk('vevent')[:max_results]:
        #     summary = event.get('summary')
        #     start_time = event.get('dtstart').dt.strftime('%Y-%m-%d %H:%M:%S')
        #     extracted_events.append({
        #         'summary': summary,
        #         'start_time': start_time
        #     })

        # return extracted_events

        
    except Exception as e:
        return JSONResponse(status_code=500, content={'message': f'Error: {str(e)}'})
