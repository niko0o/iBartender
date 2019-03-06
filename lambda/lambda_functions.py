import json
import boto3

client = boto3.client('iot-data', region_name='us-east-1')


def build_response(message, session_attributes={}):
    response = {}
    response['version'] = '1.0'
    response['sessionAttributes'] = session_attributes
    response['response'] = {'outputSpeech':message}
    return response

def build_PlainSpeech(body):
    speech = {}
    speech['type'] = 'PlainText'
    speech['text'] = body
    return speech
def lambda_handler(event, context):
    if event['request']['type'] == "LaunchRequest":
            message = build_PlainSpeech("Welcome to iBartender! P.S. Olli sux cox and dix")
            return build_response(message)
    elif event['request']['intent']['name'] == "DrinkIntent":

            #drink = ['request']['intent']['slots']['drink']['name']['value']
            message = build_PlainSpeech("One drink coming!")
            response = client.publish(
                topic='ibartender/order',
                qos=1,
                payload=json.dumps({"foo":"bar"})
            )
            return build_response(message)
