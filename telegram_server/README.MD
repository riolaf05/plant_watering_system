### Istructions

1. Update `intent.json` with responsed based on plants distributions on pots.

2. Update `pots.txt` accordingly.

3. Train the model with `chatbot_train.ipynb` or using MLflow. 

4. Upload trained model, pickle and text file on server.

5. Deploy with Kubernetes manifest (using `install.sh`) or with:

```console
docker run --restart=unless-stopped -it -e TELEGRAM_BOT_TOKEN='' -e MQTT_BROKER='' -e BROKER_USR='' -e BROKER_PSW='' -v $(pwd)/models:/src/models rio05docker/smart_garden_chatbot:v2
```