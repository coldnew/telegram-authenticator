
>> Work in Progress


# Installation

Add following to =/etc/pam.d/sshd=
```
auth       required     pam_telegram_authenticator.so
```

You may need to enable challenge-response to your sshd config, or set =/etc/ssh/sshd_config=

```
ChallengeResponseAuthentication yes
```

