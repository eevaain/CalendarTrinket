from google.oauth2 import service_account
import googleapiclient.discovery
import datetime
from fastapi import FastAPI
from fastapi.responses import JSONResponse

import datetime
from fastapi import FastAPI
from fastapi.responses import JSONResponse
import uvicorn
import os
from dotenv import load_dotenv 

# loading variables from .env file
load_dotenv() 

# Set up credentials
credentials = service_account.Credentials.from_service_account_file(
    '../etc/secrets/calendarKey.json', scopes=['https://www.googleapis.com/auth/calendar.readonly'])

# Create the Google Calendar API service
service = googleapiclient.discovery.build('calendar', 'v3', credentials=credentials)

app = FastAPI()

def get_events(calendar_id, day):
    # Define the start and end times for the day
    start_time = day.isoformat() + 'T00:00:00Z'
    end_time = day.isoformat() + 'T23:59:59Z'

    # Call the API to retrieve events
    events_result = service.events().list(calendarId=calendar_id, timeMin=start_time, timeMax=end_time,
                                          singleEvents=True, orderBy='startTime').execute()
    events = events_result.get('items', [])

    formatted_events = []
    for event in events:
        # Format start and end times to '00:00:00'
        start = event['start'].get('dateTime', '')
        if start:
            start = start.split('T')[1][:8]
        
        end = event['end'].get('dateTime', '')
        if end:
            end = end.split('T')[1][:8]

        formatted_event = {
            "summary": event.get('summary', ''),
            "start": start,
            "end": end
        }
        formatted_events.append(formatted_event)

    return formatted_events


# Example usage

@app.get("/calendars")
async def get_calendars():
    try:
        calendar_id = os.getenv("CALENDAR_ID")
        day_to_fetch = datetime.date.today() 
        # day_to_fetch = datetime.date(2024, 4, 1)  # Specify the day to fetch events
        events = get_events(calendar_id, day_to_fetch)
        return events
        
    except Exception as e:
        return JSONResponse(status_code=500, content={'message': f'Error: {str(e)}'})

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)