import React, { Component } from 'react';
import './App.css';

import {Nav, NavItem, Button, Collapse} from 'react-bootstrap';


class App extends Component
{
    constructor(...args)
    {
        super(...args);
        this.state = { showNavbar: false };

        this.onToggleNavbar.bind(this);

    }

    onToggleNavbar(e)
    {
        this.setState({...this.state, showNavbar: !this.state.showNavbar })
        console.log('toggle click', e)
        e.stopPropagation();
    }

    onGlobalClick(e)
    {
        this.setState({...this.state, showNavbar: false })
        console.log('global click', e)
    }

    render()
    {
        return (
            <div className="App" onClick={ (e) => this.onGlobalClick(e)}>
                <div className="titleWrap">
                    <div className="title label-primary">
                        <Button className="visible-xs toggleNav btn-primary" onClick={(e) => this.onToggleNavbar(e)}>
                            <span className="icon-bar"></span>
                            <span className="icon-bar"></span>
                            <span className="icon-bar"></span>
                        </Button>
                        <span>Smarthome - Sensors</span>
                    </div>
                </div>
                <div className="widthWrap">
                    <Nav className={this.state.showNavbar ? "Navbar" : "Navbar hidden-xs"} bsStyle="pills" stacked>
                        <NavItem>Sensors</NavItem>
                        <NavItem>Controls</NavItem>
                        <NavItem>Events</NavItem>
                    </Nav>
                    <div className="hidden-xs NavbarFiller">

                    </div>
                    <div className="contenWrap">
                        Администраторы сервера выбирают, какой порт будут использовать клиенты для ретрансляции исходящей почты — 25 или 587. Спецификации и многие сервера поддерживают и тот, и другой порты. Хотя некоторые сервера поддерживают порт 465 для безопасного SMTP, но предпочтительнее использовать стандартные порты и ESMTP-команды, если необходима защищённая сессия между клиентом и сервером.
                        Некоторые сервера настроены на отклонение всех ретрансляций по порту 25, но пользователям, прошедшим аутентификацию по порту 587, позволено перенаправлять сообщения на любой действительный адрес.
                        Некоторые провайдеры перехватывают порт 25, перенаправляя трафик на свой собственный SMTP-сервер вне зависимости от адреса назначения. Таким образом, их пользователи не могут получить доступ к серверу за пределами провайдерской сети по порту 25.
                        Некоторые сервера поддерживают аутентифицированный доступ по дополнительному, отличному от 25, порту, позволяя пользователям соединяться с ними, даже если порт 25 заблокирован.
                    </div>
                </div>
            </div>
        );
    }
}

export default App;
