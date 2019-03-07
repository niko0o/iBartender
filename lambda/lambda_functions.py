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
            message = build_PlainSpeech("Welcome to iBartender! Please order drink, for example, \"Alexa, ask bartender to make me a rum and coke\"")
            return build_response(message)
    elif event['request']['intent']['name'] == "DrinkIntent":
            if event['request']['intent']['slots']['drink']['resolutions']['resolutionsPerAuthority'][0]['status']['code'] == "ER_SUCCESS_NO_MATCH":
                message = build_PlainSpeech("I can't make that drink yet. Please try another drink")
                return build_response(message)
            else:
                drink = event['request']['intent']['slots']['drink']['resolutions']['resolutionsPerAuthority'][0]['values'][0]['value']['name']
                drinkId = event['request']['intent']['slots']['drink']['resolutions']['resolutionsPerAuthority'][0]['values'][0]['value']['id']
                message = build_PlainSpeech("One " + drink + " coming!")
                response = client.publish(
                    topic='ibartender/order',
                    qos=1,
                    payload=json.dumps({"drink":drinkId})
                )
                return build_response(message)
    else:
        message = build_PlainSpeech("Sorry, I didn't get that. Please order drink, for example, \"Alexa, ask bartender to make me a rum and coke\"")
        return build_response(message)
