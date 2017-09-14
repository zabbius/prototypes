# -*- coding: utf-8 -*-

import BaseHTTPServer
import SocketServer

__all__ = ['CreateHttpRequestHandlerRedirectorClass',
           'CreateTcpRequestHandlerRedirectorClass',
           'CreateUdpRequestHandlerRedirectorClass']

defaultHttpMethods = ['GET', 'POST']


def CreateHttpRequestHandlerRedirectorClass(handlerDelegate, methods = defaultHttpMethods):
    class HttpRedirector(BaseHTTPServer.BaseHTTPRequestHandler):
        def __getattr__(self, item):
            if item in HttpRedirector.supportedMethods:
                return self.doMethod
            else:
                raise AttributeError()

        def doMethod(self):
            HttpRedirector.handlerDelegate(self)

    HttpRedirector.supportedMethods = []
    for method in methods:
        HttpRedirector.supportedMethods.append('do_' + method)

    HttpRedirector.handlerDelegate = handlerDelegate
    HttpRedirector.protocol_version = "HTTP/1.1"

    return HttpRedirector


def CreateTcpRequestHandlerRedirectorClass(handlerDelegate):
    class Redirector(SocketServer.StreamRequestHandler):
        def handle(self):
            Redirector.handleRequest(self)

    Redirector.handleRequest = handlerDelegate
    return Redirector


def CreateUdpRequestHandlerRedirectorClass(handlerDelegate):
    class Redirector(SocketServer.DatagramRequestHandler):
        def handle(self):
            Redirector.handleRequest(self)

    Redirector.handleRequest = handlerDelegate
    return Redirector


