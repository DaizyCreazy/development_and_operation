from django.contrib.auth.models import User
from rest_framework import viewsets
from rest_framework import permissions
from general.app.serializers import UserSerializer, UserSerializerAnother


class UserViewSet(viewsets.ModelViewSet):
    """
    API endpoint that allows users to be viewed or edited.
    """
    queryset = User.objects.all().order_by('-date_joined')
    serializer_class = UserSerializer
    permission_classes = [permissions.IsAuthenticated]


class UserViewSetAnother(viewsets.ModelViewSet):
    """
    API endpoint that allows groups to be viewed or edited.
    """
    queryset = User.objects.all().order_by('-date_joined')
    serializer_class = UserSerializerAnother
    permission_classes = [permissions.IsAuthenticated]
