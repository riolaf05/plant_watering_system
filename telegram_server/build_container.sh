VERSION=$1
docker build -t rio05docker/smart_garden_chatbot:$VERSION .
docker push rio05docker/smart_garden_chatbot:$VERSION

